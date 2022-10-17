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

#include "autogain.hpp"

AutoGain::AutoGain(const std::string& tag,
                   const std::string& schema,
                   const std::string& schema_path,
                   PipeManager* pipe_manager)
    : PluginBase(tag, tags::plugin_name::autogain, tags::plugin_package::ebur128, schema, schema_path, pipe_manager),
      target(g_settings_get_double(settings, "target")),
      silence_threshold(g_settings_get_double(settings, "silence-threshold")) {
  reference = parse_reference_key(util::gsettings_get_string(settings, "reference"));

  gconnections.push_back(g_signal_connect(settings, "changed::target",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<AutoGain*>(user_data);

                                            self->target = g_settings_get_double(settings, key);
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::silence-threshold",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<AutoGain*>(user_data);

                                            self->silence_threshold = g_settings_get_double(settings, key);
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::maximum-history",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<AutoGain*>(user_data);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->set_maximum_history(g_settings_get_int(settings, key));
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::reset-history", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<AutoGain*>(user_data);

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

  gconnections.push_back(g_signal_connect(
      settings, "changed::reference", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
        auto self = static_cast<AutoGain*>(user_data);

        self->reference = parse_reference_key(util::gsettings_get_string(settings, key));
      }),
      this));

  setup_input_output_gain();
}

AutoGain::~AutoGain() {
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

auto AutoGain::init_ebur128() -> bool {
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

auto AutoGain::parse_reference_key(const std::string& key) -> Reference {
  if (key == "Momentary") {
    return Reference::momentary;
  }

  if (key == "Shortterm") {
    return Reference::shortterm;
  }

  if (key == "Integrated") {
    return Reference::integrated;
  }

  if (key == "Geometric Mean (MS)") {
    return Reference::geometric_mean_ms;
  }

  if (key == "Geometric Mean (MI)") {
    return Reference::geometric_mean_mi;
  }

  if (key == "Geometric Mean (SI)") {
    return Reference::geometric_mean_si;
  }

  return Reference::geometric_mean_msi;
}

void AutoGain::set_maximum_history(const int& seconds) {
  if (ebur_state == nullptr) {
    return;
  }

  // The value given to ebur128_set_max_history must be in milliseconds

  ebur128_set_max_history(ebur_state, static_cast<ulong>(seconds) * 1000UL);
}

void AutoGain::setup() {
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

void AutoGain::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ebur128_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  for (size_t n = 0U; n < n_samples; n++) {
    data[2U * n] = left_in[n];
    data[2U * n + 1U] = right_in[n];
  }

  ebur128_add_frames_float(ebur_state, data.data(), n_samples);

  auto failed = false;

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm)) {
    failed = true;
  } else if (shortterm > 10.0) {
    /*
      Sometimes when a stream is started right after EasyEffects has been initialized a very large shorterm value is
      calculated. Probably because of some weird high intensity transient. So it is better to ignore unresonable large
       values. When they happen we just set the shorterm value to the momentary loudness.
    */

    shortterm = momentary;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global)) {
    failed = true;
  } else if (global > 10.0) {
    /*
      Sometimes when a stream is started right after EasyEffects has been initialized a very large integrated value is
      calculated. Probably because of some weird high intensity transient. So it is better to ignore unresonable large
       values. When they happen we just set the global value to the momentary loudness.
    */

    global = momentary;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range)) {
    failed = true;
  }

  if (momentary > silence_threshold && !failed) {
    double peak_L = 0.0;
    double peak_R = 0.0;

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 0U, &peak_L)) {
      failed = true;
    }

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 1U, &peak_R)) {
      failed = true;
    }

    if (!failed) {
      switch (reference) {
        case Reference::momentary: {
          loudness = momentary;

          break;
        }
        case Reference::shortterm: {
          loudness = shortterm;

          break;
        }
        case Reference::integrated: {
          loudness = global;

          break;
        }
        case Reference::geometric_mean_msi: {
          loudness = std::cbrt(momentary * shortterm * global);

          break;
        }
        case Reference::geometric_mean_ms: {
          loudness = std::sqrt(std::fabs(momentary * shortterm));

          if (momentary < 0 && shortterm < 0) {
            loudness *= -1;
          }

          break;
        }
        case Reference::geometric_mean_mi: {
          loudness = std::sqrt(std::fabs(momentary * global));

          if (momentary < 0 && global < 0) {
            loudness *= -1;
          }

          break;
        }
        case Reference::geometric_mean_si: {
          loudness = std::sqrt(std::fabs(shortterm * global));

          if (shortterm < 0 && global < 0) {
            loudness *= -1;
          }

          break;
        }
      }

      const double diff = target - loudness;

      // 10^(diff/20). The way below should be faster than using pow
      const double gain = std::exp((diff / 20.0) * std::log(10.0));

      const double peak = (peak_L > peak_R) ? peak_L : peak_R;

      const auto db_peak = util::linear_to_db(peak);

      if (db_peak > util::minimum_db_level) {
        if (gain * peak < 1.0) {
          internal_output_gain = gain;
        }
      }
    }
  }

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (internal_output_gain != 1.0F) {
    apply_gain(left_out, right_out, static_cast<float>(internal_output_gain));
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += buffer_duration;

    if (notification_dt >= notification_time_window) {
      results.emit(loudness, internal_output_gain, momentary, shortterm, global, relative, range);

      notify();

      notification_dt = 0.0F;
    }
  }
}

auto AutoGain::get_latency_seconds() -> float {
  return 0.0F;
}
