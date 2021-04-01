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
    : PluginBase(tag, plugin_name::rnnoise, schema, schema_path, pipe_manager), data_L(0), data_R(0) {
  data_L.reserve(blocksize);
  data_R.reserve(blocksize);

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("model-path").connect([=, this](auto key) {
    free_rnnoise();

    auto* m = get_model_from_file();

    std::lock_guard<std::mutex> guard(rnnoise_mutex);

    model = m;

    state_left = rnnoise_create(model);
    state_right = rnnoise_create(model);
  });

  auto* m = get_model_from_file();

  model = m;

  state_left = rnnoise_create(model);
  state_right = rnnoise_create(model);
}

RNNoise::~RNNoise() {
  util::debug(log_tag + name + " destroyed");

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  free_rnnoise();
}

void RNNoise::setup() {
  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  resample = rate != rnnoise_rate;

  data_L.resize(0);
  data_R.resize(0);

  resampler_inL = std::make_unique<Resampler>(rate, rnnoise_rate);
  resampler_inR = std::make_unique<Resampler>(rate, rnnoise_rate);

  resampler_outL = std::make_unique<Resampler>(rnnoise_rate, rate);
  resampler_outR = std::make_unique<Resampler>(rnnoise_rate, rate);
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

  apply_gain(left_in, right_in, input_gain);

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  if (resample) {
    auto resampled_inL = resampler_inL->process(left_in, false);
    auto resampled_inR = resampler_inR->process(right_in, false);

    resampled_data_L.resize(0);
    resampled_data_R.resize(0);

    remove_noise(resampled_inL, resampled_inR, resampled_data_L, resampled_data_R);

    auto resampled_outL = resampler_outL->process(resampled_data_L, false);
    auto resampled_outR = resampler_outR->process(resampled_data_R, false);

    for (const auto& v : resampled_outL) {
      deque_out_L.emplace_back(v);
    }

    for (const auto& v : resampled_outR) {
      deque_out_R.emplace_back(v);
    }
  } else {
    remove_noise(left_in, right_in, deque_out_L, deque_out_R);
  }

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

      util::debug("rnnoise latency: " + std::to_string(latency) + " s");

      Glib::signal_idle().connect_once([=, this] { new_latency.emit(latency); });

      notify_latency = false;
    }
  }
}

auto RNNoise::get_model_from_file() -> RNNModel* {
  const auto* path = settings->get_string("model-path").c_str();

  RNNModel* m = nullptr;

  if (path != nullptr) {
    FILE* f = fopen(path, "r");

    if (f != nullptr) {
      util::debug("rnnoise plugin: loading model from file: " + std::string(path));

      m = rnnoise_model_from_file(f);

      fclose(f);
    }
  }

  return m;
}

void RNNoise::free_rnnoise() {
  if (state_left != nullptr) {
    rnnoise_destroy(state_left);
  }

  if (state_right != nullptr) {
    rnnoise_destroy(state_right);
  }

  if (model != nullptr) {
    rnnoise_model_free(model);
  }

  state_left = nullptr;
  state_right = nullptr;
  model = nullptr;
}

auto RNNoise::get_latency() const -> float {
  return latency;
}
