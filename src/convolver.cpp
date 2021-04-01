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

Convolver::Convolver(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_path,
                     PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::convolver, schema, schema_path, pipe_manager) {
  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("ir-width").connect([=, this](auto key) { ir_width = settings->get_int(key); });

  settings->signal_changed("model-path").connect([=, this](auto key) {
    // resampler_L = std::make_unique<Resampler>(rate, rnnoise_rate);

    std::lock_guard<std::mutex> guard(lock_guard_zita);
  });
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
}

void Convolver::setup() {
  std::lock_guard<std::mutex> guard(lock_guard_zita);
}

void Convolver::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  if (bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

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
  SndfileHandle file = SndfileHandle(settings->get_string("kernel-path"));
}

void Convolver::apply_kernel_autogain() {
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

  for (uint i = 0; i < kernel_L.size(); i++) {
    float L = kernel_L[i];
    float R = kernel_R[i];

    kernel_L[i] = L + x * R;
    kernel_R[i] = R + x * L;
  }
}

// g_settings_bind(settings, "kernel-path", convolver, "kernel-path", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "ir-width", convolver, "ir-width", G_SETTINGS_BIND_DEFAULT);
