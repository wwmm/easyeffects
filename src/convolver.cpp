/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "convolver.hpp"
#include <qnamespace.h>
#include <qstandardpaths.h>
#include <sched.h>
#include <sndfile.h>
#include <sys/types.h>
#include <zita-convolver.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <execution>
#include <filesystem>
#include <memory>
#include <mutex>
#include <numeric>
#include <sndfile.hh>
#include <span>
#include <string>
#include <tuple>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_convolver.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "resampler.hpp"
#include "tags_app.hpp"
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
      app_config_dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()) {
  init_common_controls<db::Convolver>(settings);

  // Initialize directories for local and community irs
  local_dir_irs = app_config_dir + "/irs";

  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox)
  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_irs.emplace_back("/app/extensions/Presets/irs");
  }

  // Regular paths.
  for (auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
    dir += dir.endsWith("/") ? "" : "/";

    system_data_dir_irs.push_back(dir.toStdString() + "irs");
  }

  connect(settings, &db::Convolver::irWidthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    if (kernel_is_initialized) {
      kernel_L = original_kernel_L;
      kernel_R = original_kernel_R;

      set_kernel_stereo_width();
      apply_kernel_autogain();
    }
  });

  connect(settings, &db::Convolver::kernelNameChanged, [&]() { prepare_kernel(); });

  connect(settings, &db::Convolver::autogainChanged, [&]() { prepare_kernel(); });
}

Convolver::~Convolver() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

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

  util::debug(log_tag + name.toStdString() + " destroyed");
}

void Convolver::reset() {
  settings->setDefaults();
}

void Convolver::setup() {
  ready = false;

  /*
    As zita uses fftw we have to be careful when reinitializing it. The thread that creates the fftw plan has to be the
    same that destroys it. Otherwise segmentation faults can happen. As we do not want to do this initializing in the
    plugin realtime thread we send it to the main thread through g_idle_add().connect_once
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

        data_L.resize(0U);
        data_R.resize(0U);

        deque_out_L.resize(0U);
        deque_out_R.resize(0U);

        notify_latency = true;

        latency_n_frames = 0U;

        read_kernel_file();

        if (kernel_is_initialized) {
          kernel_L = original_kernel_L;
          kernel_R = original_kernel_R;

          set_kernel_stereo_width();
          apply_kernel_autogain();

          setup_zita();
        }

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = kernel_is_initialized && zita_ready;
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
    for (size_t j = 0U; j < left_in.size(); j++) {
      data_L.push_back(left_in[j]);
      data_R.push_back(right_in[j]);

      if (data_L.size() == blocksize) {
        do_convolution(data_L, data_R);

        for (const auto& v : data_L) {
          deque_out_L.push_back(v);
        }

        for (const auto& v : data_R) {
          deque_out_R.push_back(v);
        }

        data_L.resize(0U);
        data_R.resize(0U);
      }
    }

    // copying the processed samples to the output buffers

    if (deque_out_L.size() >= left_out.size()) {
      for (float& v : left_out) {
        v = deque_out_L.front();

        deque_out_L.pop_front();
      }

      for (float& v : right_out) {
        v = deque_out_R.front();

        deque_out_R.pop_front();
      }
    } else {
      const uint offset = 2U * (left_out.size() - deque_out_L.size());

      if (offset != latency_n_frames) {
        latency_n_frames = offset;

        notify_latency = true;
      }

      for (uint n = 0U; !deque_out_L.empty() && n < left_out.size(); n++) {
        if (n < offset) {
          left_out[n] = 0.0F;
          right_out[n] = 0.0F;
        } else {
          left_out[n] = deque_out_L.front();
          right_out[n] = deque_out_R.front();

          deque_out_R.pop_front();
          deque_out_L.pop_front();
        }
      }
    }
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name.toStdString() + " latency: " + util::to_string(latency_value, "") + " s");

    update_filter_params();

    notify_latency = false;
  }

  get_peaks(left_in, right_in, left_out, right_out);
}

void Convolver::process([[maybe_unused]] std::span<float>& left_in,
                        [[maybe_unused]] std::span<float>& right_in,
                        [[maybe_unused]] std::span<float>& left_out,
                        [[maybe_unused]] std::span<float>& right_out,
                        [[maybe_unused]] std::span<float>& probe_left,
                        [[maybe_unused]] std::span<float>& probe_right) {}

auto Convolver::search_irs_path(const std::string& name) -> std::string {
  // Given the irs name without extension, search the full path on the filesystem.
  const auto irs_filename = name + irs_ext;

  const auto community_package = (pipeline_type == PipelineType::input) ? db::Main::lastLoadedInputCommunityPackage()
                                                                        : db::Main::lastLoadedOutputCommunityPackage();

  std::string irs_full_path;

  if (community_package.isEmpty()) {
    // Search local irs file.
    const auto local_irs_file = std::filesystem::path{local_dir_irs + "/" + irs_filename};

    if (std::filesystem::exists(local_irs_file)) {
      irs_full_path = local_irs_file.c_str();
    }
  } else {
    // Search irs file in community package paths.
    for (const auto& xdg_irs_dir : system_data_dir_irs) {
      if (util::search_filename(std::filesystem::path{xdg_irs_dir + "/" + community_package.toStdString()},
                                irs_filename, irs_full_path, 3U)) {
        break;
      }
    }
  }

  return irs_full_path;
}

void Convolver::read_kernel_file() {
  kernel_is_initialized = false;

  const auto name = settings->kernelName();

  if (name.isEmpty()) {
    util::warning(log_tag + name.toStdString() + ": irs filename is null. Entering passthrough mode...");

    return;
  }

  const auto path = search_irs_path(name.toStdString());

  // If the search fails, the path is empty
  if (path.empty()) {
    util::warning(log_tag + name.toStdString() + ": irs filename does not exist. Entering passthrough mode...");

    return;
  }

  util::debug("trying to load irs: " + path);

  // SndfileHandle might have issues with std::string, so we provide cstring

  SndfileHandle file = SndfileHandle(path.c_str());

  if (file.channels() == 0 || file.frames() == 0) {
    util::warning(log_tag + name.toStdString() + ": irs file does not exists or it is empty: " + path);
    util::warning(log_tag + name.toStdString() + ": Entering passthrough mode...");

    return;
  }

  util::debug(log_tag + name.toStdString() + ": irs file: " + path);
  util::debug(log_tag + name.toStdString() + ": irs rate: " + util::to_string(file.samplerate()) + " Hz");
  util::debug(log_tag + name.toStdString() + ": irs channels: " + util::to_string(file.channels()));
  util::debug(log_tag + name.toStdString() + ": irs frames: " + util::to_string(file.frames()));

  // for now only stereo irs files are supported

  if (file.channels() != 2) {
    util::warning(log_tag + name.toStdString() + " Only stereo impulse responses are supported.");
    util::warning(log_tag + name.toStdString() + " The impulse file was not loaded!");

    return;
  }

  std::vector<float> buffer(file.frames() * file.channels());
  std::vector<float> buffer_L(file.frames());
  std::vector<float> buffer_R(file.frames());

  file.readf(buffer.data(), file.frames());

  for (size_t n = 0U; n < buffer_L.size(); n++) {
    buffer_L[n] = buffer[2U * n];
    buffer_R[n] = buffer[(2U * n) + 1U];
  }

  if (file.samplerate() != static_cast<int>(rate)) {
    util::debug(log_tag + name.toStdString() + " resampling the kernel to " + util::to_string(rate));

    auto resampler = std::make_unique<Resampler>(file.samplerate(), rate);

    original_kernel_L = resampler->process(buffer_L, true);

    resampler = std::make_unique<Resampler>(file.samplerate(), rate);

    original_kernel_R = resampler->process(buffer_R, true);
  } else {
    original_kernel_L = buffer_L;
    original_kernel_R = buffer_R;
  }

  kernel_is_initialized = true;

  util::debug(log_tag + name.toStdString() + ": kernel correctly initialized");
}

void Convolver::apply_kernel_autogain() {
  if (!settings->autogain()) {
    return;
  }

  if (kernel_L.empty() || kernel_R.empty()) {
    return;
  }

  const float abs_peak_L =
      std::ranges::max(kernel_L, [](const auto& a, const auto& b) { return (std::fabs(a) < std::fabs(b)); });
  const float abs_peak_R =
      std::ranges::max(kernel_R, [](const auto& a, const auto& b) { return (std::fabs(a) < std::fabs(b)); });

  const float peak = (abs_peak_L > abs_peak_R) ? abs_peak_L : abs_peak_R;

  // normalize

  std::ranges::for_each(kernel_L, [&](auto& v) { v /= peak; });
  std::ranges::for_each(kernel_R, [&](auto& v) { v /= peak; });

  // find average power

  float power_L = 0.0F;
  float power_R = 0.0F;

  std::ranges::for_each(kernel_L, [&](const auto& v) { power_L += v * v; });
  std::ranges::for_each(kernel_R, [&](const auto& v) { power_R += v * v; });

  const float power = std::max(power_L, power_R);

  const float autogain = std::min(1.0F, 1.0F / std::sqrt(power));

  util::debug(log_tag + "autogain factor: " + util::to_string(autogain));

  std::ranges::for_each(kernel_L, [&](auto& v) { v *= autogain; });
  std::ranges::for_each(kernel_R, [&](auto& v) { v *= autogain; });
}

/*
   Mid-Side based Stereo width effect
   taken from https://github.com/tomszilagyi/ir.lv2/blob/automatable/ir.cc
*/
void Convolver::set_kernel_stereo_width() {
  const float w = static_cast<float>(settings->irWidth()) * 0.01F;
  const float x = (1.0F - w) / (1.0F + w);  // M-S coeff.; L_out = L + x*R; R_out = R + x*L

  for (uint i = 0U; i < original_kernel_L.size(); i++) {
    const auto L = original_kernel_L[i];
    const auto R = original_kernel_R[i];

    kernel_L[i] = L + x * R;
    kernel_R[i] = R + x * L;
  }
}

void Convolver::setup_zita() {
  zita_ready = false;

  if (n_samples == 0U || !kernel_is_initialized) {
    return;
  }

  const uint max_convolution_size = kernel_L.size();
  const uint buffer_size = get_zita_buffer_size();

  if (conv != nullptr) {
    conv->stop_process();

    conv->cleanup();

    delete conv;
  }

  conv = new Convproc();

  conv->set_options(0);

  int ret = conv->configure(2, 2, max_convolution_size, buffer_size, buffer_size, buffer_size, 0.0F /*density*/);

  if (ret != 0) {
    util::warning(log_tag + name.toStdString() +
                  " can't initialise zita-convolver engine: " + util::to_string(ret, ""));

    return;
  }

  ret = conv->impdata_create(0, 0, 1, kernel_L.data(), 0, static_cast<int>(kernel_L.size()));

  if (ret != 0) {
    util::warning(log_tag + name.toStdString() + " left impdata_create failed: " + util::to_string(ret));

    return;
  }

  ret = conv->impdata_create(1, 1, 1, kernel_R.data(), 0, static_cast<int>(kernel_R.size()));

  if (ret != 0) {
    util::warning(log_tag + name.toStdString() + " right impdata_create failed: " + util::to_string(ret, ""));

    return;
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    util::warning(log_tag + name.toStdString() + " start_process failed: " + util::to_string(ret, ""));

    conv->stop_process();
    conv->cleanup();

    return;
  }

  zita_ready = true;

  util::debug(log_tag + name.toStdString() + ": zita is ready");
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
  if (n_samples == 0U || rate == 0U) {
    return;
  }

  data_mutex.lock();

  ready = false;

  data_mutex.unlock();

  read_kernel_file();

  if (kernel_is_initialized) {
    kernel_L = original_kernel_L;
    kernel_R = original_kernel_R;

    set_kernel_stereo_width();
    apply_kernel_autogain();

    setup_zita();

    data_mutex.lock();

    ready = kernel_is_initialized && zita_ready;

    data_mutex.unlock();
  }
}

void Convolver::direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
  std::vector<size_t> indices(c.size());

  std::iota(indices.begin(), indices.end(), 0U);

  auto each = [&](const int n) {
    c[n] = 0.0F;

    // Static cast to avoid gcc signedness warning.

    const int a_size = static_cast<int>(a.size());
    const int b_size = static_cast<int>(b.size());

    for (int m = 0; m < b_size; m++) {
      if (const auto z = n - m; z >= 0 && z < a_size - 1) {
        c[n] += b[m] * a[z];
      }
    }
  };
#if defined(ENABLE_LIBCPP_WORKAROUNDS) && (_LIBCPP_VERSION < 170000 || defined(_LIBCPP_HAS_NO_INCOMPLETE_PSTL))
  std::for_each(indices.begin(), indices.end(), each);
#else
  std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), each);
#endif
}

auto Convolver::simple_read_kernel(const std::string& kernel_path)
    -> std::tuple<int, std::vector<float>, std::vector<float>> {
  int rate = 0;
  std::vector<float> buffer;
  std::vector<float> kernel_L;
  std::vector<float> kernel_R;

  auto file_path = std::filesystem::path{kernel_path};

  util::debug("reading the impulse file: " + file_path.string());

  if (file_path.extension() != irs_ext) {
    file_path += irs_ext;
  }

  if (!std::filesystem::exists(file_path)) {
    util::debug("file: " + file_path.string() + " does not exist");

    return std::make_tuple(rate, kernel_L, kernel_R);
  }

  auto sndfile = SndfileHandle(file_path.string());

  if (sndfile.channels() != 2 || sndfile.frames() == 0) {
    util::warning(" Only stereo impulse responses are supported.");
    util::warning(" The impulse file was not loaded!");

    return std::make_tuple(rate, kernel_L, kernel_R);
  }

  buffer.resize(sndfile.frames() * sndfile.channels());
  kernel_L.resize(sndfile.frames());
  kernel_R.resize(sndfile.frames());

  sndfile.readf(buffer.data(), sndfile.frames());

  for (size_t n = 0U; n < kernel_L.size(); n++) {
    kernel_L[n] = buffer[2U * n];
    kernel_R[n] = buffer[(2U * n) + 1U];
  }

  rate = sndfile.samplerate();

  return std::make_tuple(rate, kernel_L, kernel_R);
}

void Convolver::combine_kernels(const std::string& kernel_1_name,
                                const std::string& kernel_2_name,
                                const std::string& output_file_name) {
  if (output_file_name.empty()) {
    // The method combine_kernels run in a secondary thread. But the widgets have to be used in the main thread.

    // g_object_ref(self);

    // util::idle_add([=] { gtk_spinner_stop(self->spinner); }, [=]() { g_object_unref(self); });

    return;
  }

  const auto kernel_1_path = search_irs_path(kernel_1_name);
  const auto kernel_2_path = search_irs_path(kernel_2_name);

  // If the search fails, the path is empty
  if (kernel_1_path.empty()) {
    util::warning(log_tag + kernel_1_path + ": irs filename does not exist.");

    return;
  }

  if (kernel_2_path.empty()) {
    util::warning(log_tag + kernel_2_path + ": irs filename does not exist.");

    return;
  }

  auto [rate1, kernel_1_L, kernel_1_R] = simple_read_kernel(kernel_1_path);
  auto [rate2, kernel_2_L, kernel_2_R] = simple_read_kernel(kernel_2_path);

  if (rate1 == 0 || rate2 == 0) {
    // g_object_ref(self);

    // util::idle_add([=] { gtk_spinner_stop(self->spinner); }, [=]() { g_object_unref(self); });

    return;
  }

  if (rate1 > rate2) {
    util::debug("resampling the kernel " + kernel_2_name + " to " + util::to_string(rate1) + " Hz");

    auto resampler = std::make_unique<Resampler>(rate2, rate1);

    kernel_2_L = resampler->process(kernel_2_L, true);

    resampler = std::make_unique<Resampler>(rate2, rate1);

    kernel_2_R = resampler->process(kernel_2_R, true);
  } else if (rate2 > rate1) {
    util::debug("resampling the kernel " + kernel_1_name + " to " + util::to_string(rate2) + " Hz");

    auto resampler = std::make_unique<Resampler>(rate1, rate2);

    kernel_1_L = resampler->process(kernel_1_L, true);

    resampler = std::make_unique<Resampler>(rate1, rate2);

    kernel_1_R = resampler->process(kernel_1_R, true);
  }

  std::vector<float> kernel_L(kernel_1_L.size() + kernel_2_L.size() - 1U);
  std::vector<float> kernel_R(kernel_1_R.size() + kernel_2_R.size() - 1U);

  // As the convolution is commutative we change the order based on which will run faster.

  if (kernel_1_L.size() > kernel_2_L.size()) {
    direct_conv(kernel_1_L, kernel_2_L, kernel_L);
    direct_conv(kernel_1_R, kernel_2_R, kernel_R);
  } else {
    direct_conv(kernel_2_L, kernel_1_L, kernel_L);
    direct_conv(kernel_2_R, kernel_1_R, kernel_R);
  }

  std::vector<float> buffer(kernel_L.size() * 2U);  // 2 channels interleaved

  for (size_t n = 0U; n < kernel_L.size(); n++) {
    buffer[2U * n] = kernel_L[n];
    buffer[(2U * n) + 1U] = kernel_R[n];
  }

  const auto output_file_path = local_dir_irs / std::filesystem::path{output_file_name + irs_ext};

  auto mode = SFM_WRITE;
  auto format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
  auto n_channels = 2;
  auto rate = (rate1 > rate2) ? rate1 : rate2;

  auto sndfile = SndfileHandle(output_file_path.string(), mode, format, n_channels, rate);

  sndfile.writef(buffer.data(), static_cast<sf_count_t>(kernel_L.size()));

  util::debug("combined kernel saved: " + output_file_path.string());

  // g_object_ref(self);

  // util::idle_add([=] { gtk_spinner_stop(self->spinner); }, [=]() { g_object_unref(self); });
}
