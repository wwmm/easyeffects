/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include "level_meter.hpp"

LevelMeter::LevelMeter(const std::string& tag,
                       const std::string& schema,
                       const std::string& schema_path,
                       PipeManager* pipe_manager)
    : PluginBase(tag,
                 tags::plugin_name::level_meter,
                 tags::plugin_package::ebur128,
                 schema,
                 schema_path,
                 pipe_manager) {
  gconnections.push_back(g_signal_connect(settings, "changed::maximum-history",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<LevelMeter*>(user_data);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->set_maximum_history(g_settings_get_int(settings, key));
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::reset-history", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto* self = static_cast<LevelMeter*>(user_data);

        self->mythreads.emplace_back([self]() {  // Using emplace_back here makes sense
          self->data_mutex.lock();

          self->ebur128_ready = false;

          self->data_mutex.unlock();

          auto status = self->init_ebur128();

          self->data_mutex.lock();

          self->ebur128_ready = status;

          self->data_mutex.unlock();
        });
      }),
      this));
}

LevelMeter::~LevelMeter() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  for (auto& t : mythreads) {
    t.join();
  }

  mythreads.clear();

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);
  }

  util::debug(log_tag + name + " destroyed");
}

auto LevelMeter::init_ebur128() -> bool {
  if (n_samples == 0 || rate == 0) {
    return false;
  }

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);

    ebur_state = nullptr;
  }

  ebur_state = ebur128_init(2U, rate, EBUR128_MODE_S | EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_SAMPLE_PEAK);

  ebur128_set_channel(ebur_state, 0U, EBUR128_LEFT);
  ebur128_set_channel(ebur_state, 1U, EBUR128_RIGHT);

  set_maximum_history(g_settings_get_int(settings, "maximum-history"));

  return ebur_state != nullptr;
}

void LevelMeter::set_maximum_history(const int& seconds) {
  if (ebur_state == nullptr) {
    return;
  }

  // The value given to ebur128_set_max_history must be in milliseconds

  ebur128_set_max_history(ebur_state, static_cast<ulong>(seconds) * 1000UL);
}

void LevelMeter::setup() {
  if (2U * static_cast<size_t>(n_samples) != data.size()) {
    data.resize(static_cast<size_t>(n_samples) * 2U);
  }

  if (rate != old_rate) {
    data_mutex.lock();

    ebur128_ready = false;

    data_mutex.unlock();

    mythreads.emplace_back([this]() {  // Using emplace_back here makes sense
      if (ebur128_ready) {
        return;
      }

      auto status = true;

      old_rate = rate;

      status = init_ebur128();

      data_mutex.lock();

      ebur128_ready = status;

      data_mutex.unlock();
    });
  }
}

void LevelMeter::process(std::span<float>& left_in,
                         std::span<float>& right_in,
                         std::span<float>& left_out,
                         std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ebur128_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  for (size_t n = 0U; n < n_samples; n++) {
    data[2U * n] = left_in[n];
    data[2U * n + 1U] = right_in[n];
  }

  ebur128_add_frames_float(ebur_state, data.data(), n_samples);

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary)) {
    momentary = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm)) {
    shortterm = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global)) {
    global = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative)) {
    relative = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range)) {
    range = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 0U, &sample_peak_L)) {
    sample_peak_L = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 1U, &sample_peak_R)) {
    sample_peak_R = 0.0;
  }

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    if (send_notifications) {
      results.emit(momentary, shortterm, global, relative, range, sample_peak_L, sample_peak_R);

      notify();
    }
  }
}

auto LevelMeter::get_latency_seconds() -> float {
  return 0.0F;
}
