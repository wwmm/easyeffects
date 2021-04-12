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

#include "crystalizer.hpp"

Crystalizer::Crystalizer(const std::string& tag,
                         const std::string& schema,
                         const std::string& schema_path,
                         PipeManager* pipe_manager)
    : PluginBase(tag, "crystalizer", schema, schema_path, pipe_manager) {
  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  initialize_listener();
}

Crystalizer::~Crystalizer() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  std::lock_guard<std::mutex> lock(data_mutex);

  futures.clear();
}

void Crystalizer::setup() {
  data_mutex.lock();

  filters_are_ready = false;

  data_mutex.unlock();

  n_samples_is_power_of_2 = (n_samples & (n_samples - 1)) == 0 && n_samples != 0;

  if (!n_samples_is_power_of_2) {
    blocksize = n_samples;

    while ((blocksize & (blocksize - 1)) != 0 && blocksize > 2) {
      blocksize--;
    }

    util::debug("convolver blocksize: " + std::to_string(blocksize));
  }

  data_L.resize(0);
  data_R.resize(0);

  notify_latency = true;

  latency_n_frames = 0;

  // auto f = [=, this]() { setup_zita(); };

  // auto future = std::async(std::launch::async, f);

  // futures.emplace_back(std::move(future));
}

void Crystalizer::process(std::span<float>& left_in,
                          std::span<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out) {
  std::lock_guard<std::mutex> lock(data_mutex);

  if (bypass || !filters_are_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  if (n_samples_is_power_of_2) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    // do_convolution(left_out, right_out);
  } else {
    for (size_t j = 0; j < left_in.size(); j++) {
      data_L.emplace_back(left_in[j]);
      data_R.emplace_back(right_in[j]);

      if (data_L.size() == blocksize) {
        // do_convolution(data_L, data_R);

        for (const auto& v : data_L) {
          deque_out_L.emplace_back(v);
        }

        for (const auto& v : data_R) {
          deque_out_R.emplace_back(v);
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
      uint offset = 2 * (left_out.size() - deque_out_L.size());

      if (offset != latency_n_frames) {
        latency_n_frames = offset;

        notify_latency = true;
      }

      for (uint n = 0; !deque_out_L.empty() && n < left_out.size(); n++) {
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

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }

    if (notify_latency) {
      latency = static_cast<float>(latency_n_frames) / rate;

      util::debug(name + " latency: " + std::to_string(latency) + " s");

      Glib::signal_idle().connect_once([=, this] { new_latency.emit(latency); });

      notify_latency = false;
    }
  }
}

// g_settings_bind(settings, "post-messages", crystalizer, "notify-host", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "aggressive", crystalizer, "aggressive", G_SETTINGS_BIND_DEFAULT);

// for (int n = 0; n < 13; n++) {
//   g_settings_bind_with_mapping(settings, std::string("intensity-band" + std::to_string(n)).c_str(), crystalizer,
//                                std::string("intensity-band" + std::to_string(n)).c_str(), G_SETTINGS_BIND_GET,
//                                util::db20_gain_to_linear, nullptr, nullptr, nullptr);

//   g_settings_bind(settings, std::string("mute-band" + std::to_string(n)).c_str(), crystalizer,
//                   std::string("mute-band" + std::to_string(n)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings, std::string("bypass-band" + std::to_string(n)).c_str(), crystalizer,
//                   std::string("bypass-band" + std::to_string(n)).c_str(), G_SETTINGS_BIND_DEFAULT);
// }
