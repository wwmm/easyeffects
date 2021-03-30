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

#include "rnnoise.hpp"

RNNoise::RNNoise(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::rnnoise, schema, schema_path, pipe_manager), data_L(0), data_R(0) {}

RNNoise::~RNNoise() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  std::lock_guard<std::mutex> guard(rnnoise_mutex);
}

void RNNoise::setup() {
  resample = rate != rnnoise_rate;

  data_L.resize(0);
  data_R.resize(0);

  resampler_inL = std::make_unique<Resampler>(rate, rnnoise_rate);
  resampler_inR = std::make_unique<Resampler>(rate, rnnoise_rate);

  resampler_outL = std::make_unique<Resampler>(rnnoise_rate, rate);
  resampler_outR = std::make_unique<Resampler>(rnnoise_rate, rate);

  std::lock_guard<std::mutex> guard(rnnoise_mutex);
}

void RNNoise::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  if (bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  if (resample) {
    auto resampled_inL = resampler_inL->process(left_in, false);
    auto resampled_inR = resampler_inR->process(right_in, false);

    auto resampled_outL = resampler_outL->process(resampled_inL, false);
    auto resampled_outR = resampler_outR->process(resampled_inR, false);

    for (const auto& v : resampled_outL) {
      deque_out_L.emplace_back(v);
    }

    for (const auto& v : resampled_outR) {
      deque_out_R.emplace_back(v);
    }
  } else {
    for (const auto& v : left_in) {
      data_L.emplace_back(v);

      if (data_L.size() == blocksize) {
        // rnnoise_process_frame(state_left, data_L.data(), data_L.data());

        for (const auto& v : data_L) {
          deque_out_L.emplace_back(v);
        }

        data_L.resize(0);
      }
    }

    for (const auto& v : right_in) {
      data_R.push_back(v);

      if (data_R.size() == blocksize) {
        // rnnoise_process_frame(state_left, data_L.data(), data_L.data());

        for (const auto& v : data_R) {
          deque_out_R.emplace_back(v);
        }

        data_R.resize(0);
      }
    }
  }

  // util::warning(std::to_string(left_in.size()) + ", " + std::to_string(resampled_inL.size()) + ", " +
  //               std::to_string(deque_out_L.size()));

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

    latency = static_cast<float>(offset) / rate;

    util::debug("rnnoise latency: " + std::to_string(latency) + " s");

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

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

auto RNNoise::get_latency() const -> float {
  return latency;
}

// g_settings_bind(settings, "model-path", rnnoise, "model-path", G_SETTINGS_BIND_DEFAULT);
