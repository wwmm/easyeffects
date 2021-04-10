/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
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
    : PluginBase(tag, plugin_name::convolver, schema, schema_path, pipe_manager), conv(new Convproc()) {
  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("ir-width").connect([=, this](auto key) {
    ir_width = settings->get_int(key);

    std::lock_guard<std::mutex> lock(lock_guard_zita);

    if (kernel_is_initialized) {
      kernel_L = original_kernel_L;
      kernel_R = original_kernel_R;

      set_kernel_stereo_width();
      apply_kernel_autogain();
    }
  });

  settings->signal_changed("kernel-path").connect([=, this](auto key) {
    lock_guard_zita.lock();

    kernel_is_initialized = false;
    zita_ready = false;

    lock_guard_zita.unlock();

    read_kernel_file();

    if (kernel_is_initialized) {
      kernel_L = original_kernel_L;
      kernel_R = original_kernel_R;

      set_kernel_stereo_width();
      apply_kernel_autogain();

      setup_zita();
    }
  });

  initialize_listener();
}

Convolver::~Convolver() {
  util::debug(log_tag + name + " destroyed");

  std::lock_guard<std::mutex> lock(lock_guard_zita);

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  futures.clear();

  conv->stop_process();
  conv->cleanup();

  delete conv;
}

void Convolver::setup() {
  lock_guard_zita.lock();

  kernel_is_initialized = false;
  zita_ready = false;

  lock_guard_zita.unlock();

  util::warning(std::to_string(n_samples));

  n_samples_is_power_of_2 = (n_samples & (n_samples - 1)) == 0 && n_samples != 0;

  if (!n_samples_is_power_of_2) {
    blocksize = n_samples;

    while ((blocksize & (blocksize - 1)) == 0 && blocksize > 2) {
      blocksize--;
    }

    data_L.resize(0);
    data_R.resize(0);
  }

  auto f = [=, this]() {
    read_kernel_file();

    if (kernel_is_initialized) {
      kernel_L = original_kernel_L;
      kernel_R = original_kernel_R;

      set_kernel_stereo_width();
      apply_kernel_autogain();

      setup_zita();
    }
  };

  auto future = std::async(std::launch::async, f);

  futures.emplace_back(std::move(future));
}

void Convolver::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::lock_guard<std::mutex> lock(lock_guard_zita);

  if (bypass || !kernel_is_initialized || !zita_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  if (n_samples_is_power_of_2) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    do_convolution(left_out, right_out);
  } else {
  }

  apply_gain(left_out, right_out, output_gain);

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

  auto path = settings->get_string("kernel-path");

  if (path.c_str() == nullptr) {
    util::warning(log_tag + name + ": irs file path is null. Entering passthrough mode...");

    return;
  }

  SndfileHandle file = SndfileHandle(path);

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

  for (size_t n = 0; n < buffer_L.size(); n++) {
    buffer_L[n] = buffer[2 * n];
    buffer_R[n] = buffer[2 * n + 1];
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

  float abs_peak_L = std::ranges::max(kernel_L, [](auto& a, auto& b) { return (std::fabs(a) < std::fabs(b)); });
  float abs_peak_R = std::ranges::max(kernel_R, [](auto& a, auto& b) { return (std::fabs(a) < std::fabs(b)); });

  float peak = (abs_peak_L > abs_peak_R) ? abs_peak_L : abs_peak_R;

  // normalize

  std::ranges::for_each(kernel_L, [&](auto& v) { v /= peak; });
  std::ranges::for_each(kernel_R, [&](auto& v) { v /= peak; });

  // find average power

  float power = 0.0F;

  std::ranges::for_each(kernel_L, [&](auto& v) { power += v * v; });
  std::ranges::for_each(kernel_R, [&](auto& v) { power += v * v; });

  power *= 0.5F;

  float autogain = std::min(1.0F, 1.0F / sqrtf(power));

  util::debug(log_tag + "autogain factor: " + std::to_string(autogain));

  std::ranges::for_each(kernel_L, [&](auto& v) { v *= autogain; });
  std::ranges::for_each(kernel_R, [&](auto& v) { v *= autogain; });
}

/*
   Mid-Side based Stereo width effect
   taken from https://github.com/tomszilagyi/ir.lv2/blob/automatable/ir.cc
*/
void Convolver::set_kernel_stereo_width() {
  float w = ir_width * 0.01F;
  float x = (1.0F - w) / (1.0F + w);  // M-S coeff.; L_out = L + x*R; R_out = R + x*L

  for (uint i = 0; i < original_kernel_L.size(); i++) {
    float L = original_kernel_L[i];
    float R = original_kernel_R[i];

    kernel_L[i] = L + x * R;
    kernel_R[i] = R + x * L;
  }
}

void Convolver::setup_zita() {
  zita_ready = false;

  if (rate == 0 || n_samples == 0 || !kernel_is_initialized) {
    return;
  }

  conv->stop_process();
  conv->cleanup();

  int ret = 0;
  int max_convolution_size = kernel_L.size();
  int buffer_size = get_zita_buffer_size();
  uint options = 0;
  float density = 0.0F;

  options |= Convproc::OPT_VECTOR_MODE;

  conv->set_options(options);

#if ZITA_CONVOLVER_MAJOR_VERSION == 3
  conv->set_density(density);

  ret = conv->configure(2, 2, max_convolution_size, buffer_size, buffer_size, buffer_size);
#endif

#if ZITA_CONVOLVER_MAJOR_VERSION == 4
  ret = conv->configure(2, 2, max_convolution_size, buffer_size, buffer_size, buffer_size, density);
#endif

  if (ret != 0) {
    util::warning(log_tag + "can't initialise zita-convolver engine: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(0, 0, 1, kernel_L.data(), 0, kernel_L.size());

  if (ret != 0) {
    util::debug(log_tag + "left impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(1, 1, 1, kernel_R.data(), 0, kernel_R.size());

  if (ret != 0) {
    util::debug(log_tag + "right impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    util::debug(log_tag + "start_process failed: " + std::to_string(ret));

    conv->stop_process();
    conv->cleanup();

    return;
  }

  zita_ready = true;

  util::debug(log_tag + name + ": zita is ready");
}

auto Convolver::get_zita_buffer_size() -> int {
  if (n_samples_is_power_of_2) {
    return n_samples;
  }

  return blocksize;
}