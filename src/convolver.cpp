/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "convolver.hpp"

namespace {

constexpr auto CONVPROC_SCHEDULER_PRIORITY = 0;

constexpr auto CONVPROC_SCHEDULER_CLASS = SCHED_FIFO;

}  // namespace

Convolver::Convolver(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_path,
                     PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::convolver, schema, schema_path, pipe_manager) {
  settings->signal_changed("ir-width").connect([=, this](const auto& key) {
    ir_width = settings->get_int(key);

    std::scoped_lock<std::mutex> lock(data_mutex);

    if (kernel_is_initialized) {
      kernel_L = original_kernel_L;
      kernel_R = original_kernel_R;

      set_kernel_stereo_width();
      apply_kernel_autogain();
    }
  });

  settings->signal_changed("kernel-path").connect([=, this](const auto& key) {
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
  });

  setup_input_output_gain();
}

Convolver::~Convolver() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  if (conv != nullptr) {
    conv->stop_process();

    conv->cleanup();

    delete conv;
  }

  util::debug(log_tag + name + " destroyed");
}

void Convolver::setup() {
  ready = false;

  /*
    As zita uses fftw we have to be careful when reinitializing it. The thread that creates the fftw plan has to be the
    same that destroys it. Otherwise segmentation faults can happen. As we do not want to do this initializing in the
    plugin realtime thread we send it to the main thread through Glib::signal_idle().connect_once
  */

  Glib::signal_idle().connect_once([&, this] {
    blocksize = n_samples;

    n_samples_is_power_of_2 = (n_samples & (n_samples - 1)) == 0 && n_samples != 0;

    if (!n_samples_is_power_of_2) {
      while ((blocksize & (blocksize - 1)) != 0 && blocksize > 2) {
        blocksize--;
      }
    }

    data_L.resize(0);
    data_R.resize(0);

    deque_out_L.resize(0);
    deque_out_R.resize(0);

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
  });
}

void Convolver::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (n_samples_is_power_of_2) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

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

        data_L.resize(0);
        data_R.resize(0);
      }
    }

    // copying the precessed samples to the output buffers

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
    const float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + std::to_string(latency_value) + " s");

    Glib::signal_idle().connect_once([=, this] { latency.emit(latency_value); });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_value * 1000000000.0F);

    std::array<char, 1024U> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);

    notify_latency = false;
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

void Convolver::read_kernel_file() {
  kernel_is_initialized = false;

  const auto& path = settings->get_string("kernel-path");

  if (path.empty()) {
    util::warning(log_tag + name + ": irs file path is null. Entering passthrough mode...");

    return;
  }

  // SndfileHandle might have issues with std::string, so we provide cstring

  SndfileHandle file = SndfileHandle(path.c_str());

  if (file.channels() == 0 || file.frames() == 0) {
    util::warning(log_tag + name + ": irs file does not exists or it is empty: " + path);
    util::warning(log_tag + name + ": Entering passthrough mode...");

    return;
  }

  util::debug(log_tag + name + ": irs file: " + path);
  util::debug(log_tag + name + ": irs rate: " + std::to_string(file.samplerate()) + " Hz");
  util::debug(log_tag + name + ": irs channels: " + std::to_string(file.channels()));
  util::debug(log_tag + name + ": irs frames: " + std::to_string(file.frames()));

  // for now only stereo irs files are supported

  if (file.channels() != 2) {
    util::warning(log_tag + name + " Only stereo impulse responses are supported.");
    util::warning(log_tag + name + " The impulse file was not loaded!");

    return;
  }

  std::vector<float> buffer(file.frames() * file.channels());
  std::vector<float> buffer_L(file.frames());
  std::vector<float> buffer_R(file.frames());

  file.readf(buffer.data(), file.frames());

  for (size_t n = 0U; n < buffer_L.size(); n++) {
    buffer_L[n] = buffer[2U * n];
    buffer_R[n] = buffer[2U * n + 1U];
  }

  if (file.samplerate() != static_cast<int>(rate)) {
    util::debug(log_tag + name + " resampling the kernel to " + std::to_string(rate));

    auto resampler = std::make_unique<Resampler>(file.samplerate(), rate);

    original_kernel_L = resampler->process(buffer_L, true);

    resampler = std::make_unique<Resampler>(file.samplerate(), rate);

    original_kernel_R = resampler->process(buffer_R, true);
  } else {
    original_kernel_L = buffer_L;
    original_kernel_R = buffer_R;
  }

  kernel_is_initialized = true;

  util::debug(log_tag + name + ": kernel initialized");
}

void Convolver::apply_kernel_autogain() {
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

  float power = 0.0F;

  std::ranges::for_each(kernel_L, [&](const auto& v) { power += v * v; });
  std::ranges::for_each(kernel_R, [&](const auto& v) { power += v * v; });

  power *= 0.5F;

  const float autogain = std::min(1.0F, 1.0F / std::sqrt(power));

  util::debug(log_tag + "autogain factor: " + std::to_string(autogain));

  std::ranges::for_each(kernel_L, [&](auto& v) { v *= autogain; });
  std::ranges::for_each(kernel_R, [&](auto& v) { v *= autogain; });
}

/*
   Mid-Side based Stereo width effect
   taken from https://github.com/tomszilagyi/ir.lv2/blob/automatable/ir.cc
*/
void Convolver::set_kernel_stereo_width() {
  const float w = static_cast<float>(ir_width) * 0.01F;
  const float x = (1.0F - w) / (1.0F + w);  // M-S coeff.; L_out = L + x*R; R_out = R + x*L

  for (uint i = 0U; i < original_kernel_L.size(); i++) {
    const auto& L = original_kernel_L[i];
    const auto& R = original_kernel_R[i];

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

  if (conv == nullptr) {
    conv = new Convproc();
  }

  conv->stop_process();

  conv->cleanup();

  conv->set_options(0);

  int ret = conv->configure(2, 2, max_convolution_size, buffer_size, buffer_size, buffer_size, 0.0F /*density*/);

  if (ret != 0) {
    util::warning(log_tag + name + " can't initialise zita-convolver engine: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(0, 0, 1, kernel_L.data(), 0, static_cast<int>(kernel_L.size()));

  if (ret != 0) {
    util::warning(log_tag + name + " left impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(1, 1, 1, kernel_R.data(), 0, static_cast<int>(kernel_R.size()));

  if (ret != 0) {
    util::warning(log_tag + name + " right impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    util::warning(log_tag + name + " start_process failed: " + std::to_string(ret));

    conv->stop_process();
    conv->cleanup();

    return;
  }

  zita_ready = true;

  util::debug(log_tag + name + ": zita is ready");
}

auto Convolver::get_zita_buffer_size() -> uint {
  if (n_samples_is_power_of_2) {
    return n_samples;
  }

  return blocksize;
}
