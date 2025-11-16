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
#include <sndfile.hh>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "convolver_kernel_manager.hpp"
#include "db_manager.hpp"
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

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
  destructor_called = true;
  ready = false;

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  disconnect();
  settings->disconnect();

  std::scoped_lock<std::mutex> lock(data_mutex);

  zita.stop();

  for (auto& t : mythreads) {
    t.join();
  }

  mythreads.clear();

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
        if (ready || destructor_called) {
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

    zita.process(left_out, right_out);
  } else {
    buf_in_L.insert(buf_in_L.end(), left_in.begin(), left_in.end());
    buf_in_R.insert(buf_in_R.end(), right_in.begin(), right_in.end());

    while (buf_in_L.size() >= blocksize) {
      util::copy_bulk(buf_in_L, data_L);
      util::copy_bulk(buf_in_R, data_R);

      zita.process(data_L, data_R);

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

  auto magL = util::interpolate(time_axis, copy_helper, x_linear);

  std::ranges::copy(kernel.right_channel, copy_helper.begin());

  auto magR = util::interpolate(time_axis, copy_helper, x_linear);

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

  auto left_copy = kernel.left_channel;
  auto right_copy = kernel.right_channel;
  auto rate_copy = kernel.rate;

  mythreads.emplace_back([this, left_copy = std::move(left_copy), right_copy = std::move(right_copy),
                          rate_copy = rate_copy]() { chart_kernel_fft(left_copy, right_copy, rate_copy); });
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

    auto success = zita.init(kernel.sampleCount(), blocksize, kernel.left_channel, kernel.right_channel);

    if (!success) {
      util::warning(std::format("{} Zita init failed", log_tag));
    }

    data_mutex.lock();

    ready = kernel_is_initialized && success;

    data_mutex.unlock();
  }
}

void Convolver::combine_kernels(const std::string& kernel_1_name,
                                const std::string& kernel_2_name,
                                const std::string& output_file_name) {
  kernel_manager.combineKernels(kernel_1_name, kernel_2_name, output_file_name);

  QMetaObject::invokeMethod(this, [this] { Q_EMIT kernelCombinationStopped(); }, Qt::QueuedConnection);
}

void Convolver::combineKernels(const QString& kernel1, const QString& kernel2, const QString& outputName) {
  mythreads.emplace_back(  // Using emplace_back here makes sense
      [this, kernel1, kernel2, outputName]() {
        combine_kernels(kernel1.toStdString(), kernel2.toStdString(), outputName.toStdString());
      });
}

void Convolver::chart_kernel_fft(const std::vector<float>& kernel_L,
                                 const std::vector<float>& kernel_R,
                                 const float& kernel_rate) {
  kernel_fft.calculate_fft(kernel_L, kernel_R, kernel_rate, interpPoints);

  QMetaObject::invokeMethod(
      this,
      [this] {
        if (!ready || destructor_called) {
          return;
        }

        auto linear_L = kernel_fft.linear_L;
        auto linear_R = kernel_fft.linear_R;
        auto log_L = kernel_fft.log_L;
        auto log_R = kernel_fft.log_R;

        chartMagLfftLinear.swap(linear_L);
        chartMagRfftLinear.swap(linear_R);
        chartMagLfftLog.swap(log_L);
        chartMagRfftLog.swap(log_R);

        Q_EMIT chartMagLfftLinearChanged();
        Q_EMIT chartMagRfftLinearChanged();
        Q_EMIT chartMagLfftLogChanged();
        Q_EMIT chartMagRfftLogChanged();
      },
      Qt::QueuedConnection);
}

void Convolver::clear_chart_data() {
  chartMagL.clear();
  chartMagR.clear();
  chartMagLfftLinear.clear();
  chartMagRfftLinear.clear();
  chartMagLfftLog.clear();
  chartMagRfftLog.clear();
}
