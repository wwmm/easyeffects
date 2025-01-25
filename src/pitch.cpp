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

#include "pitch.hpp"
#include <STTypes.h>
#include <SoundTouch.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <algorithm>
#include <cstddef>
#include <mutex>
#include <span>
#include <string>
#include "pipe_manager.hpp"
#include "plugin_base.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Pitch::Pitch(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager,
             PipelineType pipe_type)
    : PluginBase(tag,
                 tags::plugin_name::pitch,
                 tags::plugin_package::sound_touch,
                 schema,
                 schema_path,
                 pipe_manager,
                 pipe_type) {
  quick_seek = g_settings_get_boolean(settings, "quick-seek") != 0;
  anti_alias = g_settings_get_boolean(settings, "anti-alias") != 0;

  sequence_length_ms = g_settings_get_int(settings, "sequence-length");
  seek_window_ms = g_settings_get_int(settings, "seek-window");
  overlap_length_ms = g_settings_get_int(settings, "overlap-length");

  tempo_difference = g_settings_get_double(settings, "tempo-difference");
  rate_difference = g_settings_get_double(settings, "rate-difference");

  semitones = g_settings_get_double(settings, "semitones");

  // resetting soundtouch when bypass is pressed so its internal data is discarded

  gconnections.push_back(g_signal_connect(settings, "changed::bypass",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            util::idle_add([&, self] {
                                              self->data_mutex.lock();

                                              self->soundtouch_ready = false;

                                              self->data_mutex.unlock();

                                              self->init_soundtouch();

                                              self->data_mutex.lock();

                                              self->soundtouch_ready = true;

                                              self->data_mutex.unlock();
                                            });
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::quick-seek",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->quick_seek = g_settings_get_boolean(settings, key) != 0;

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_quick_seek();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::anti-alias",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->anti_alias = g_settings_get_boolean(settings, key) != 0;

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_quick_seek();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::sequence-length",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->sequence_length_ms = g_settings_get_int(settings, key);

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_sequence_length();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::seek-window",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->seek_window_ms = g_settings_get_int(settings, key);

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_seek_window();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::overlap-length",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->overlap_length_ms = g_settings_get_int(settings, key);

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_overlap_length();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::tempo-difference",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->tempo_difference = g_settings_get_double(settings, key);

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_tempo_difference();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::rate-difference",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->rate_difference = g_settings_get_double(settings, key);

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_rate_difference();
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::semitones",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto* self = static_cast<Pitch*>(user_data);

                                            self->semitones = g_settings_get_double(settings, key);

                                            if (!self->soundtouch_ready) {
                                              return;
                                            }

                                            self->set_semitones();
                                          }),
                                          this));

  setup_input_output_gain();
}

Pitch::~Pitch() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Pitch::setup() {
  soundtouch_ready = false;

  latency_n_frames = 0U;

  if (data.size() != static_cast<size_t>(n_samples) * 2) {
    data.resize(2U * static_cast<size_t>(n_samples));
  }

  deque_out_L.resize(0U);
  deque_out_R.resize(0U);

  util::idle_add([&, this] {
    if (soundtouch_ready) {
      return;
    }

    init_soundtouch();

    std::scoped_lock<std::mutex> lock(data_mutex);

    soundtouch_ready = true;
  });
}

void Pitch::process(std::span<float>& left_in,
                    std::span<float>& right_in,
                    std::span<float>& left_out,
                    std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !soundtouch_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  for (size_t n = 0U; n < left_in.size(); n++) {
    data[n * 2U] = left_in[n];
    data[n * 2U + 1U] = right_in[n];
  }

  snd_touch->putSamples(data.data(), n_samples);

  uint n_received = 0U;

  do {
    n_received = snd_touch->receiveSamples(data.data(), n_samples);

    for (size_t n = 0U; n < n_received; n++) {
      deque_out_L.push_back(data[n * 2U]);
      deque_out_R.push_back(data[n * 2U + 1U]);
    }

  } while (n_received != 0);

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
    const uint offset = left_out.size() - deque_out_L.size();

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

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + util::to_string(latency_value, "") + " s");

    util::idle_add([this]() {
      if (!post_messages || latency.empty()) {
        return;
      }

      latency.emit();
    });

    update_filter_params();

    notify_latency = false;
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    if (send_notifications) {
      notify();
    }
  }
}

void Pitch::set_semitones() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setPitchSemiTones(semitones);
}

void Pitch::set_sequence_length() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_SEQUENCE_MS, sequence_length_ms);
}

void Pitch::set_seek_window() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_SEEKWINDOW_MS, seek_window_ms);
}

void Pitch::set_overlap_length() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_OVERLAP_MS, overlap_length_ms);
}

void Pitch::set_quick_seek() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_USE_QUICKSEEK, static_cast<int>(quick_seek));
}

void Pitch::set_anti_alias() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_USE_AA_FILTER, static_cast<int>(anti_alias));
}

void Pitch::set_tempo_difference() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setTempoChange(tempo_difference);
}

void Pitch::set_rate_difference() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setRateChange(rate_difference);
}

void Pitch::init_soundtouch() {
  delete snd_touch;

  snd_touch = new soundtouch::SoundTouch();

  snd_touch->setSampleRate(rate);
  snd_touch->setChannels(2);

  set_semitones();
  set_quick_seek();
  set_anti_alias();
  set_sequence_length();
  set_seek_window();
  set_overlap_length();
  set_tempo_difference();
  set_rate_difference();
}

auto Pitch::get_latency_seconds() -> float {
  return latency_value;
}
