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

#include "autogain.hpp"

AutoGain::AutoGain(const std::string& tag,
                   const std::string& schema,
                   const std::string& schema_path,
                   PipeManager* pipe_manager)
    : PluginBase(tag, "autogain", schema, schema_path, pipe_manager) {
  //   g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
  //                                util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr,
  //                                nullptr);

  //   g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
  //                                util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr,
  //                                nullptr);
}

AutoGain::~AutoGain() {
  util::debug(log_tag + name + " destroyed");

  std::lock_guard<std::mutex> lock(my_lock_guard);

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);
  }
}

void AutoGain::setup() {
  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);

    ebur_state = nullptr;
  }

  ebur_state = ebur128_init(
      2U, rate, EBUR128_MODE_S | EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_SAMPLE_PEAK | EBUR128_MODE_HISTOGRAM);

  ebur128_set_channel(ebur_state, 0U, EBUR128_LEFT);
  ebur128_set_channel(ebur_state, 1U, EBUR128_RIGHT);

  data.resize(n_samples * 2);
}

void AutoGain::process(const std::vector<float>& left_in,
                       const std::vector<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  if (bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  for (uint n = 0; n < n_samples; n++) {
    data[2 * n] = left_in[n];
    data[2 * n + 1] = right_in[n];
  }

  std::lock_guard<std::mutex> lock(my_lock_guard);

  ebur128_add_frames_float(ebur_state, data.data(), n_samples);

  bool failed = false;
  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;
  double range = 0.0;
  double loudness = 0.0F;

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range)) {
    failed = true;
  }

  if (relative > -70.0F && !failed) {
    double peak_L = 0.0;
    double peak_R = 0.0;

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 0U, &peak_L)) {
      failed = true;
    }

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 1U, &peak_R)) {
      failed = true;
    }

    if (!failed) {
      loudness = std::cbrt(momentary * shortterm * global);

      double diff = target - loudness;

      // 10^(diff/20). The way below should be faster than using pow
      double gain = exp((diff / 20.0) * log(10.0));

      double peak = (peak_L > peak_R) ? peak_L : peak_R;

      double db_peak = util::linear_to_db(peak);

      if (db_peak > util::minimum_db_level) {
        if (gain * peak < 1.0F) {
          output_gain = gain;
        }
      }
    }
  }

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  std::transform(left_out.begin(), left_out.end(), left_out.begin(), [=, this](float& c) { return c * output_gain; });

  std::transform(right_out.begin(), right_out.end(), right_out.begin(),
                 [=, this](float& c) { return c * output_gain; });

  notification_dt += static_cast<float>(n_samples) / rate;

  if (notification_dt > notification_time_window) {
    notification_dt = 0.0F;

    Glib::signal_idle().connect_once(
        [=, this] { results.emit(loudness, output_gain, momentary, shortterm, global, relative, range); });
  }
}

// void AutoGain::bind_to_gsettings() {
//   // g_settings_bind_with_mapping(settings, "target", autogain, "target", G_SETTINGS_BIND_GET, util::double_to_float,
//   //                              nullptr, nullptr, nullptr);

//   // g_settings_bind(settings, "weight-m", autogain, "weight-m", G_SETTINGS_BIND_DEFAULT);

//   // g_settings_bind(settings, "weight-s", autogain, "weight-s", G_SETTINGS_BIND_DEFAULT);

//   // g_settings_bind(settings, "weight-i", autogain, "weight-i", G_SETTINGS_BIND_DEFAULT);

//   // g_settings_bind(settings, "detect-silence", autogain, "detect-silence", G_SETTINGS_BIND_DEFAULT);

//   // g_settings_bind(settings, "use-geometric-mean", autogain, "use-geometric-mean", G_SETTINGS_BIND_DEFAULT);

//   // g_settings_bind(settings, "reset", autogain, "reset", G_SETTINGS_BIND_DEFAULT);
// }
