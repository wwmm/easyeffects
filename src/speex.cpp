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
      data_L(0),
      data_R(0) {

  gconnections.push_back(g_signal_connect(settings, "changed::noise-suppression",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Speex*>(user_data);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->noise_suppression = g_settings_get_int(settings, key);

#ifdef SPEEX_AVAILABLE
                                            if (self->state_left) {
                                              speex_preprocess_ctl(self->state_left, SPEEX_PREPROCESS_SET_DENOISE, &self->noise_suppression);
                                            }
                                            
                                            if (self->state_right) {
                                              speex_preprocess_ctl(self->state_right, SPEEX_PREPROCESS_SET_DENOISE, &self->noise_suppression);
                                            }
#endif
                                          }),
                                          this));

  
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

  blocksize = 0.001F * static_cast<float>(blocksize_ms * rate);

  data_L.resize(0);
  data_R.resize(0);

  deque_out_L.resize(0);
  deque_out_R.resize(0);

  speex_ready = false;

#ifdef SPEEX_AVAILABLE
  if (state_left != nullptr) {
    speex_preprocess_state_destroy(state_left);
  }

  if (state_right != nullptr) {
    speex_preprocess_state_destroy(state_right);
  }

  printf("blocksize: %u\n", blocksize);

  state_left = speex_preprocess_state_init(blocksize, rate);
  state_right = speex_preprocess_state_init(blocksize, rate);

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
  for (size_t i = 0; i < left_in.size(); i++) {
    data_L.push_back(left_in[i] * static_cast<float>(SHRT_MAX + 1));
    data_R.push_back(right_in[i] * static_cast<float>(SHRT_MAX + 1));

    if (data_L.size() == blocksize) {
      speex_preprocess_run(state_left, data_L.data());
      speex_preprocess_run(state_right, data_R.data());

      for (const auto& v : data_L) {
        deque_out_L.push_back(static_cast<float>(v) * inv_short_max);
      }

      for (const auto& v : data_R) {
        deque_out_R.push_back(static_cast<float>(v) * inv_short_max);
      }

      data_L.resize(0U);
      data_R.resize(0U);
    }
  }
#endif

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

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + util::to_string(latency_value, "") + " s");

    util::idle_add([=, this]() {
      if (!post_messages || latency.empty()) {
        return;
      }

      latency.emit();
    });

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
