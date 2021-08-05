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

#include "echo_canceller.hpp"

EchoCanceller::EchoCanceller(const std::string& tag,
                             const std::string& schema,
                             const std::string& schema_path,
                             PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::echo_canceller, schema, schema_path, pipe_manager, true) {
  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("frame-size").connect([=, this](auto key) {
    std::scoped_lock<std::mutex> lock(data_mutex);

    blocksize_ms = settings->get_int(key);

    init_speex();
  });

  settings->signal_changed("filter-length").connect([=, this](auto key) {
    std::scoped_lock<std::mutex> lock(data_mutex);

    filter_length_ms = settings->get_int(key);

    init_speex();
  });
}

EchoCanceller::~EchoCanceller() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  data_mutex.lock();

  ready = false;

  if (echo_state_L != nullptr) {
    speex_echo_state_destroy(echo_state_L);
  }

  if (echo_state_R != nullptr) {
    speex_echo_state_destroy(echo_state_R);
  }

  data_mutex.unlock();

  util::debug(log_tag + name + " destroyed");
}

void EchoCanceller::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  notify_latency = true;

  latency_n_frames = 0U;

  deque_out_L.resize(0);
  deque_out_R.resize(0);

  init_speex();
}

void EchoCanceller::process(std::span<float>& left_in,
                            std::span<float>& right_in,
                            std::span<float>& left_out,
                            std::span<float>& right_out,
                            std::span<float>& probe_left,
                            std::span<float>& probe_right) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  for (size_t j = 0; j < left_in.size(); j++) {
    data_L.emplace_back(left_in[j] * (SHRT_MAX + 1));
    data_R.emplace_back(right_in[j] * (SHRT_MAX + 1));

    probe_L.emplace_back(probe_left[j] * (SHRT_MAX + 1));
    probe_R.emplace_back(probe_right[j] * (SHRT_MAX + 1));

    if (data_L.size() == blocksize) {
      speex_echo_cancellation(echo_state_L, data_L.data(), probe_L.data(), filtered_L.data());
      speex_echo_cancellation(echo_state_R, data_R.data(), probe_R.data(), filtered_R.data());

      for (const auto& v : filtered_L) {
        deque_out_L.emplace_back(static_cast<float>(v) * inv_short_max);
      }

      for (const auto& v : filtered_R) {
        deque_out_R.emplace_back(static_cast<float>(v) * inv_short_max);
      }

      data_L.resize(0);
      data_R.resize(0);
      probe_L.resize(0);
      probe_R.resize(0);
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
    uint offset = left_out.size() - deque_out_L.size();

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

  apply_gain(left_out, right_out, output_gain);

  if (notify_latency) {
    float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + std::to_string(latency_value) + " s");

    Glib::signal_idle().connect_once([=, this] { latency.emit(latency_value); });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_value * 1000000000.0F);

    std::array<char, 1024> buffer{};

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

void EchoCanceller::init_speex() {
  if (n_samples == 0U || rate == 0U) {
    return;
  }

  data_L.resize(0);
  data_R.resize(0);
  probe_L.resize(0);
  probe_R.resize(0);

  blocksize = 0.001F * blocksize_ms * rate;

  util::debug(log_tag + name + " blocksize: " + std::to_string(blocksize));

  filtered_L.resize(blocksize);
  filtered_R.resize(blocksize);

  uint filter_length = 0.001F * filter_length_ms * rate;

  util::debug(log_tag + name + " filter length: " + std::to_string(filter_length));

  if (echo_state_L != nullptr) {
    speex_echo_state_destroy(echo_state_L);
  }

  echo_state_L = speex_echo_state_init(blocksize, filter_length);

  if (speex_echo_ctl(echo_state_L, SPEEX_ECHO_SET_SAMPLING_RATE, &rate) != 0) {
    util::warning(log_tag + name + "SPEEX_ECHO_SET_SAMPLING_RATE: unknown request");
  }

  if (echo_state_R != nullptr) {
    speex_echo_state_destroy(echo_state_R);
  }

  echo_state_R = speex_echo_state_init(blocksize, filter_length);

  if (speex_echo_ctl(echo_state_R, SPEEX_ECHO_SET_SAMPLING_RATE, &rate) != 0) {
    util::warning(log_tag + name + "SPEEX_ECHO_SET_SAMPLING_RATE: unknown request");
  }

  ready = true;
}
