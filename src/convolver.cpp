/**
 * Copyright © 2017-2025 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "convolver.hpp"
#include <fftw3.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <sched.h>
#include <sndfile.h>
#include <sys/types.h>
#include <zita-convolver.h>
#include <QString>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <format>
#include <mutex>
#include <numbers>
#include <sndfile.hh>
#include <span>
#include <string>
#include <vector>
#include "convolver_kernel_manager.hpp"
#include "db_manager.hpp"
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace {

constexpr auto CONVPROC_SCHEDULER_PRIORITY = 0;

constexpr auto CONVPROC_SCHEDULER_CLASS = SCHED_FIFO;

}  // namespace

Convolver::Convolver(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::convolver,
                 tags::plugin_package::Package::zita,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Convolver>(pipe_type,
                                                           tags::plugin_name::BaseName::convolver + "#" + instance_id)),
      kernel_manager(ConvolverKernelManager(pipe_type)) {
  /**
   * Setting valid rate and n_samples values instead of zero allows the
   * convolver ui to properly show the impulse response file parameters
   * even if nothing is playing audio.
   */
  util::str_to_num(pw::Manager::self().defaultClockRate.toStdString(), rate);
  util::str_to_num(pw::Manager::self().defaultQuantum.toStdString(), n_samples);

  init_common_controls<db::Convolver>(settings);

  prepare_kernel();

  dry = (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));

  wet = (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));

  connect(settings, &db::Convolver::irWidthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    if (kernel_is_initialized) {
      kernel = original_kernel;

      set_kernel_stereo_width();
      apply_kernel_autogain();
    }
  });

  connect(settings, &db::Convolver::kernelNameChanged, [&]() { prepare_kernel(); });

  connect(settings, &db::Convolver::autogainChanged, [&]() { prepare_kernel(); });

  connect(settings, &db::Convolver::dryChanged, [&]() {
    dry =
        (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));
  });

  connect(settings, &db::Convolver::wetChanged, [&]() {
    wet =
        (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));
  });
}

Convolver::~Convolver() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  for (auto& t : mythreads) {
    t.join();
  }

  mythreads.clear();

  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  if (conv != nullptr) {
    conv->stop_process();

    conv->cleanup();

    delete conv;
  }

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Convolver::reset() {
  settings->setDefaults();
}

void Convolver::setup() {
  ready = false;

  /**
   * As zita uses fftw we have to be careful when reinitializing it.
   * The thread that creates the fftw plan has to be the same that destroys it.
   * Otherwise segmentation faults can happen. As we do not want to do this
   * initializing in the plugin realtime thread we send it to the main thread
   * through g_idle_add().connect_once
   */

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

  QMetaObject::invokeMethod(
      this,
      [this] {
        if (ready) {
          return;
        }

        blocksize = n_samples;

        n_samples_is_power_of_2 = (n_samples & (n_samples - 1U)) == 0U && n_samples != 0U;

        if (!n_samples_is_power_of_2) {
          while ((blocksize & (blocksize - 1)) != 0 && blocksize > 2) {
            blocksize--;
          }
        }

        buf_in_L.clear();
        buf_in_R.clear();
        buf_out_L.clear();
        buf_out_R.clear();

        data_L.resize(blocksize);
        data_R.resize(blocksize);

        notify_latency = true;

        latency_n_frames = 0U;

        prepare_kernel();
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Convolver::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (n_samples_is_power_of_2) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    do_convolution(left_out, right_out);
  } else {
    buf_in_L.insert(buf_in_L.end(), left_in.begin(), left_in.end());
    buf_in_R.insert(buf_in_R.end(), right_in.begin(), right_in.end());

    while (buf_in_L.size() >= blocksize) {
      util::copy_bulk(buf_in_L, data_L);
      util::copy_bulk(buf_in_R, data_R);

      do_convolution(data_L, data_R);

      buf_out_L.insert(buf_out_L.end(), data_L.begin(), data_L.end());
      buf_out_R.insert(buf_out_R.end(), data_R.begin(), data_R.end());
    }

    // copying the processed samples to the output buffers

    if (buf_out_L.size() >= n_samples) {
      util::copy_bulk(buf_out_L, left_out);
      util::copy_bulk(buf_out_R, right_out);
    } else {
      const uint offset = n_samples - buf_out_L.size();

      if (offset != latency_n_frames) {
        latency_n_frames = offset;

        notify_latency = true;
      }

      // Fill beginning with zeros
      std::fill_n(left_out.begin(), offset, 0.0F);
      std::fill_n(right_out.begin(), offset, 0.0F);

      std::ranges::copy(buf_out_L, left_out.begin() + offset);
      std::ranges::copy(buf_out_R, right_out.begin() + offset);

      buf_out_L.clear();
      buf_out_R.clear();
    }
  }

  for (size_t n = 0; n < left_out.size(); n++) {
    left_out[n] = (wet * left_out[n]) + (dry * left_in[n]);

    right_out[n] = (wet * right_out[n]) + (dry * right_in[n]);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Convolver::process([[maybe_unused]] std::span<float>& left_in,
                        [[maybe_unused]] std::span<float>& right_in,
                        [[maybe_unused]] std::span<float>& left_out,
                        [[maybe_unused]] std::span<float>& right_out,
                        [[maybe_unused]] std::span<float>& probe_left,
                        [[maybe_unused]] std::span<float>& probe_right) {}

void Convolver::load_kernel_file() {
  kernel_is_initialized = false;

  const auto name = settings->kernelName();

  kernel = kernel_manager.loadKernel(name.toStdString());

  if (!kernel.isValid()) {
    clear_chart_data();

    Q_EMIT newKernelLoaded(name, false);

    util::warning(
        std::format("{}{}: irs filename is invalid. Entering passthrough mode...", log_tag, name.toStdString()));

    return;
  }

  if (kernel.rate != static_cast<int>(rate)) {
    util::debug(
        std::format("{}{} kernel has {} rate. Resampling it to {}", log_tag, name.toStdString(), kernel.rate, rate));

    original_kernel = ConvolverKernelManager::resampleKernel(kernel, rate);
  } else {
    original_kernel = kernel;
  }

  const auto dt = 1.0 / rate;

  kernelRate = QString::fromStdString(util::to_string(kernel.rate));
  kernelSamples = QString::fromStdString(util::to_string(kernel.sampleCount()));
  kernelDuration = QString::fromStdString(util::to_string(kernel.duration()));

  std::vector<double> time_axis(kernel.sampleCount());

  for (size_t n = 0U; n < time_axis.size(); n++) {
    time_axis[n] = static_cast<double>(n) * dt;
  }

  auto x_linear = util::linspace(time_axis.front(), time_axis.back(), interpPoints);

  std::vector<double> copy_helper(kernel.sampleCount());

  std::ranges::copy(kernel.left_channel, copy_helper.begin());

  auto magL = interpolate(time_axis, copy_helper, x_linear);

  std::ranges::copy(kernel.right_channel, copy_helper.begin());

  auto magR = interpolate(time_axis, copy_helper, x_linear);

  chartMagL.resize(interpPoints);
  chartMagR.resize(interpPoints);

  for (qsizetype n = 0; n < interpPoints; n++) {
    chartMagL[n] = QPointF(x_linear[n], magL[n]);
    chartMagR[n] = QPointF(x_linear[n], magR[n]);
  }

  kernel_is_initialized = true;

  Q_EMIT newKernelLoaded(name, true);

  Q_EMIT kernelRateChanged();
  Q_EMIT kernelDurationChanged();
  Q_EMIT kernelSamplesChanged();

  Q_EMIT chartMagLChanged();
  Q_EMIT chartMagRChanged();

  util::debug(std::format("{}{}: kernel correctly loaded", log_tag, name.toStdString()));

  mythreads.emplace_back(  // Using emplace_back here makes sense
      [this]() { chart_kernel_fft(kernel.left_channel, kernel.right_channel, kernel.rate); });
}

void Convolver::apply_kernel_autogain() {
  if (!settings->autogain()) {
    return;
  }

  if (!kernel.isValid()) {
    return;
  }

  ConvolverKernelManager::normalizeKernel(kernel);

  // find average power

  float power_L = 0.0F;
  float power_R = 0.0F;

  std::ranges::for_each(kernel.left_channel, [&](const auto& v) { power_L += v * v; });
  std::ranges::for_each(kernel.right_channel, [&](const auto& v) { power_R += v * v; });

  const float power = std::max(power_L, power_R);

  const float autogain = std::min(1.0F, 1.0F / std::sqrt(power));

  util::debug(std::format("{} autogain factor: {}", log_tag, autogain));

  std::ranges::for_each(kernel.left_channel, [&](auto& v) { v *= autogain; });
  std::ranges::for_each(kernel.right_channel, [&](auto& v) { v *= autogain; });
}

/**
 * Mid-Side based Stereo width effect
 * taken from https://github.com/tomszilagyi/ir.lv2/blob/automatable/ir.cc
 */
void Convolver::set_kernel_stereo_width() {
  const float w = static_cast<float>(settings->irWidth()) * 0.01F;
  const float x = (1.0F - w) / (1.0F + w);  // M-S coeff.; L_out = L + x*R; R_out = R + x*L

  for (uint i = 0U; i < original_kernel.sampleCount(); i++) {
    const auto L = original_kernel.left_channel[i];
    const auto R = original_kernel.right_channel[i];

    kernel.left_channel[i] = L + (x * R);
    kernel.right_channel[i] = R + (x * L);
  }
}

void Convolver::setup_zita() {
  zita_ready = false;

  if (n_samples == 0U || !kernel_is_initialized) {
    return;
  }

  const uint max_convolution_size = kernel.sampleCount();
  const uint buffer_size = get_zita_buffer_size();

  if (conv != nullptr) {
    conv->stop_process();

    conv->cleanup();

    delete conv;
  }

  conv = new Convproc();

  conv->set_options(0);

  int ret = conv->configure(2, 2, max_convolution_size, buffer_size, buffer_size, buffer_size, 0.0F /* density */);

  if (ret != 0) {
    util::warning(std::format("{}can't initialise zita-convolver engine: {}", log_tag, ret));

    return;
  }

  ret = conv->impdata_create(0, 0, 1, kernel.left_channel.data(), 0, static_cast<int>(kernel.left_channel.size()));

  if (ret != 0) {
    util::warning(std::format("{}left impdata_create failed: {}", log_tag, ret));

    return;
  }

  ret = conv->impdata_create(1, 1, 1, kernel.right_channel.data(), 0, static_cast<int>(kernel.right_channel.size()));

  if (ret != 0) {
    util::warning(std::format("{}right impdata_create failed: {}", log_tag, ret));

    return;
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    util::warning(std::format("{}right impdata_create failed: {}", log_tag, ret));

    conv->stop_process();
    conv->cleanup();

    return;
  }

  zita_ready = true;

  util::debug(std::format("{}{}: zita is ready", log_tag, name.toStdString()));
}

auto Convolver::get_zita_buffer_size() -> uint {
  if (n_samples_is_power_of_2) {
    return n_samples;
  }

  return blocksize;
}

auto Convolver::get_latency_seconds() -> float {
  return this->latency_value;
}

void Convolver::prepare_kernel() {
  data_mutex.lock();

  ready = false;

  data_mutex.unlock();

  load_kernel_file();

  if (kernel_is_initialized) {
    kernel = original_kernel;

    set_kernel_stereo_width();
    apply_kernel_autogain();

    setup_zita();

    data_mutex.lock();

    ready = kernel_is_initialized && zita_ready;

    data_mutex.unlock();
  }
}

void Convolver::combine_kernels(const std::string& kernel_1_name,
                                const std::string& kernel_2_name,
                                const std::string& output_file_name) {
  kernel_manager.combineKernels(kernel_1_name, kernel_2_name, output_file_name);

  Q_EMIT kernelCombinationStopped();
}

void Convolver::combineKernels(const QString& kernel1, const QString& kernel2, const QString& outputName) {
  mythreads.emplace_back(  // Using emplace_back here makes sense
      [this, kernel1, kernel2, outputName]() {
        combine_kernels(kernel1.toStdString(), kernel2.toStdString(), outputName.toStdString());
      });
}

auto Convolver::interpolate(const std::vector<double>& x_source,
                            const std::vector<double>& y_source,
                            const std::vector<double>& x_new) -> std::vector<double> {
  auto* acc = gsl_interp_accel_alloc();
  auto* spline = gsl_spline_alloc(gsl_interp_steffen, x_source.size());

  gsl_spline_init(spline, x_source.data(), y_source.data(), x_source.size());

  std::vector<double> output(x_new.size());

  for (size_t n = 0; n < x_new.size(); n++) {
    output[n] = static_cast<float>(gsl_spline_eval(spline, x_new[n], acc));
  }

  gsl_spline_free(spline);
  gsl_interp_accel_free(acc);

  return output;
}

void Convolver::chart_kernel_fft(const std::vector<float>& kernel_L,
                                 const std::vector<float>& kernel_R,
                                 const float& kernel_rate) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (kernel_L.empty() || kernel_R.empty() || kernel_L.size() != kernel_R.size()) {
    util::debug("Aborting the impulse fft calculation...");

    return;
  }

  util::debug("Calculating the impulse fft...");

  std::vector<double> spectrum_L((kernel_L.size() / 2U) + 1U);
  std::vector<double> spectrum_R((kernel_R.size() / 2U) + 1U);

  std::vector<double> real_input(kernel_L.size());

  std::ranges::copy(kernel_L, real_input.begin());

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  auto* complex_output = fftw_alloc_complex(real_input.size());

  auto* plan =
      fftw_plan_dft_r2c_1d(static_cast<int>(real_input.size()), real_input.data(), complex_output, FFTW_ESTIMATE);

  fftw_execute(plan);

  for (uint i = 0U; i < spectrum_L.size(); i++) {
    double sqr = (complex_output[i][0] * complex_output[i][0]) + (complex_output[i][1] * complex_output[i][1]);

    sqr /= static_cast<double>(spectrum_L.size() * spectrum_L.size());

    spectrum_L[i] = sqr;
  }

  // right channel fft

  real_input.resize(kernel_R.size());

  std::ranges::copy(kernel_R, real_input.begin());

  for (uint n = 0U; n < real_input.size(); n++) {
    // https://en.wikipedia.org/wiki/Hann_function

    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(real_input.size() - 1U)));

    real_input[n] *= w;
  }

  fftw_execute(plan);

  for (uint i = 0U; i < spectrum_R.size(); i++) {
    double sqr = (complex_output[i][0] * complex_output[i][0]) + (complex_output[i][1] * complex_output[i][1]);

    sqr /= static_cast<double>(spectrum_R.size() * spectrum_R.size());

    spectrum_R[i] = sqr;
  }

  // cleaning

  if (complex_output != nullptr) {
    fftw_free(complex_output);
  }

  fftw_destroy_plan(plan);

  // initializing the frequency axis

  std::vector<double> freq_axis(spectrum_L.size());

  for (uint n = 0U; n < freq_axis.size(); n++) {
    freq_axis[n] =
        0.5F * static_cast<float>(kernel_rate) * static_cast<float>(n) / static_cast<float>(freq_axis.size());
  }

  // removing the DC component at f = 0 Hz

  freq_axis.erase(freq_axis.begin());
  spectrum_L.erase(spectrum_L.begin());
  spectrum_R.erase(spectrum_R.begin());

  // initilizing the linear axis

  auto linear_freq_axis = util::linspace(freq_axis.front(), freq_axis.back(), interpPoints);

  auto linear_spectrum_L = interpolate(freq_axis, spectrum_L, linear_freq_axis);
  auto linear_spectrum_R = interpolate(freq_axis, spectrum_R, linear_freq_axis);

  // initializing the logarithmic frequency axis

  auto max_freq = std::ranges::max(freq_axis);
  auto min_freq = std::ranges::min(freq_axis);

  util::debug(std::format("Min fft frequency: {}", min_freq));
  util::debug(std::format("Max fft frequency: {}", max_freq));

  auto log_freq_axis = util::logspace(min_freq, max_freq, interpPoints);

  auto log_spectrum_L = interpolate(freq_axis, spectrum_L, log_freq_axis);
  auto log_spectrum_R = interpolate(freq_axis, spectrum_R, log_freq_axis);

  {
    auto min_left = std::ranges::min(linear_spectrum_L);
    auto max_left = std::ranges::max(linear_spectrum_L);
    auto min_right = std::ranges::min(linear_spectrum_R);
    auto max_right = std::ranges::max(linear_spectrum_R);

    for (int n = 0U; n < interpPoints; n++) {
      linear_spectrum_L[n] = (linear_spectrum_L[n] - min_left) / (max_left - min_left);
      linear_spectrum_R[n] = (linear_spectrum_R[n] - min_right) / (max_right - min_right);
    }
  }

  {
    auto min_left = std::ranges::min(log_spectrum_L);
    auto max_left = std::ranges::max(log_spectrum_L);
    auto min_right = std::ranges::min(log_spectrum_R);
    auto max_right = std::ranges::max(log_spectrum_R);

    for (int n = 0U; n < interpPoints; n++) {
      log_spectrum_L[n] = (log_spectrum_L[n] - min_left) / (max_left - min_left);
      log_spectrum_R[n] = (log_spectrum_R[n] - min_right) / (max_right - min_right);
    }
  }

  chartMagLfftLinear.resize(interpPoints);
  chartMagRfftLinear.resize(interpPoints);
  chartMagLfftLog.resize(interpPoints);
  chartMagRfftLog.resize(interpPoints);

  for (qsizetype n = 0; n < interpPoints; n++) {
    chartMagLfftLinear[n] = QPointF(linear_freq_axis[n], linear_spectrum_L[n]);
    chartMagRfftLinear[n] = QPointF(linear_freq_axis[n], linear_spectrum_R[n]);

    chartMagLfftLog[n] = QPointF(log_freq_axis[n], log_spectrum_L[n]);
    chartMagRfftLog[n] = QPointF(log_freq_axis[n], log_spectrum_R[n]);
  }

  Q_EMIT chartMagLfftLinearChanged();
  Q_EMIT chartMagRfftLinearChanged();
  Q_EMIT chartMagLfftLogChanged();
  Q_EMIT chartMagRfftLogChanged();
}

void Convolver::clear_chart_data() {
  chartMagL.clear();
  chartMagR.clear();
  chartMagLfftLinear.clear();
  chartMagRfftLinear.clear();
  chartMagLfftLog.clear();
  chartMagRfftLog.clear();
}
