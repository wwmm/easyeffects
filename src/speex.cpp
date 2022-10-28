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

#include "speex.hpp"

Speex::Speex(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager)
    : PluginBase(tag, tags::plugin_name::speex, tags::plugin_package::speex, schema, schema_path, pipe_manager),
      enable_denoise(g_settings_get_boolean(settings, "enable-denoise")),
      noise_suppression(g_settings_get_int(settings, "noise-suppression")),
      enable_agc(g_settings_get_boolean(settings, "enable-agc")),
      enable_vad(g_settings_get_boolean(settings, "enable-vad")),
      vad_probability_start(g_settings_get_int(settings, "vad-probability-start")),
      vad_probability_continue(g_settings_get_int(settings, "vad-probability-continue")),
      enable_dereverb(g_settings_get_boolean(settings, "enable-dereverb")) {
#ifdef SPEEX_AVAILABLE

  gconnections.push_back(g_signal_connect(
      settings, "changed::enable-denoise", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
        std::scoped_lock<std::mutex> lock(self->data_mutex);

        self->enable_denoise = g_settings_get_boolean(settings, key);

        if (self->state_left) {
          speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_DENOISE, &self->enable_denoise);
        }

        if (self->state_right) {
          speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_DENOISE, &self->enable_denoise);
        }
      }),
      this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::noise-suppression", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
        std::scoped_lock<std::mutex> lock(self->data_mutex);

        self->noise_suppression = g_settings_get_int(settings, key);

        if (self->state_left) {
          speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &self->noise_suppression);
        }

        if (self->state_right) {
          speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &self->noise_suppression);
        }
      }),
      this));

  gconnections.push_back(
      g_signal_connect(settings, "changed::enable-agc", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
                         std::scoped_lock<std::mutex> lock(self->data_mutex);

                         self->enable_agc = g_settings_get_boolean(settings, key);

                         if (self->state_left) {
                           speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_AGC, &self->enable_agc);
                         }

                         if (self->state_right) {
                           speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_AGC, &self->enable_agc);
                         }
                       }),
                       this));

  gconnections.push_back(
      g_signal_connect(settings, "changed::enable-vad", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
                         std::scoped_lock<std::mutex> lock(self->data_mutex);

                         self->enable_vad = g_settings_get_boolean(settings, key);

                         if (self->state_left) {
                           speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_VAD, &self->enable_vad);
                         }

                         if (self->state_right) {
                           speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_VAD, &self->enable_vad);
                         }
                       }),
                       this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::vad-probability-start", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
        std::scoped_lock<std::mutex> lock(self->data_mutex);

        self->vad_probability_start = g_settings_get_int(settings, key);

        if (self->state_left) {
          speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_PROB_START, &self->vad_probability_start);
        }

        if (self->state_right) {
          speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_PROB_START, &self->vad_probability_start);
        }
      }),
      this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::vad-probability-continue", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
        std::scoped_lock<std::mutex> lock(self->data_mutex);

        self->vad_probability_continue = g_settings_get_int(settings, key);

        if (self->state_left) {
          speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &self->vad_probability_continue);
        }

        if (self->state_right) {
          speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &self->vad_probability_continue);
        }
      }),
      this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::enable-dereverb", G_CALLBACK(+[](GSettings* settings, char* key, Speex* self) {
        std::scoped_lock<std::mutex> lock(self->data_mutex);

        self->enable_dereverb = g_settings_get_boolean(settings, key);

        if (self->state_left) {
          speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_DEREVERB, &self->enable_dereverb);
        }

        if (self->state_right) {
          speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_DEREVERB, &self->enable_dereverb);
        }
      }),
      this));

#endif

  setup_input_output_gain();
}

Speex::~Speex() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

#ifdef SPEEX_AVAILABLE
  free_speex();
#endif

  util::debug(log_tag + name + " destroyed");
}

void Speex::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  latency_n_frames = 0U;

  speex_ready = false;

  data_L.resize(n_samples);
  data_R.resize(n_samples);

#ifdef SPEEX_AVAILABLE
  if (state_left != nullptr) {
    speex_preprocess_state_destroy(state_left);
  }

  if (state_right != nullptr) {
    speex_preprocess_state_destroy(state_right);
  }

  state_left = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));
  state_right = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));

  if (state_left != nullptr) {
    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_DENOISE, &enable_denoise);
    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppression);

    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_AGC, &enable_agc);

    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_VAD, &enable_vad);
    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_PROB_START, &vad_probability_start);
    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad_probability_continue);

    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_DEREVERB, &enable_dereverb);
  }

  if (state_right != nullptr) {
    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_DENOISE, &enable_denoise);
    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppression);

    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_AGC, &enable_agc);

    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_VAD, &enable_vad);
    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_PROB_START, &vad_probability_start);
    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad_probability_continue);

    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_DEREVERB, &enable_dereverb);
  }

  speex_ready = true;
#else
  util::warning("The Speex library was not available at compilation time. The noise reduction filter won't work");
#endif
}

void Speex::process(std::span<float>& left_in,
                    std::span<float>& right_in,
                    std::span<float>& left_out,
                    std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !speex_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

#ifdef SPEEX_AVAILABLE

  for (size_t i = 0; i < n_samples; i++) {
    data_L[i] = static_cast<spx_int16_t>(left_in[i] * (SHRT_MAX + 1));

    data_R[i] = static_cast<spx_int16_t>(right_in[i] * (SHRT_MAX + 1));
  }

  if (speex_preprocess_run(state_left, data_L.data()) == 1) {
    for (size_t i = 0; i < n_samples; i++) {
      left_out[i] = static_cast<float>(data_L[i]) * inv_short_max;
    }
  } else {
    std::ranges::fill(left_out, 0.0F);
  }

  if (speex_preprocess_run(state_right, data_R.data()) == 1) {
    for (size_t i = 0; i < n_samples; i++) {
      right_out[i] = static_cast<float>(data_R[i]) * inv_short_max;
    }
  } else {
    std::ranges::fill(right_out, 0.0F);
  }

#endif

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += buffer_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

#ifdef SPEEX_AVAILABLE

void Speex::free_speex() {
  if (state_left != nullptr) {
    speex_preprocess_state_destroy(state_left);
  }

  if (state_right != nullptr) {
    speex_preprocess_state_destroy(state_right);
  }

  state_left = nullptr;
  state_right = nullptr;
}

#endif

auto Speex::get_latency_seconds() -> float {
  return latency_value;
}
