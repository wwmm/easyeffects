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

#include "crystalizer.hpp"

Crystalizer::Crystalizer(const std::string& tag,
                         const std::string& schema,
                         const std::string& schema_path,
                         PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::crystalizer, schema, schema_path, pipe_manager) {
  for (uint n = 0U; n < nbands; n++) {
    filters.at(n) = std::make_unique<FirFilterBandpass>(log_tag + name + " band" + std::to_string(n));
  }

  std::ranges::fill(band_mute, false);
  std::ranges::fill(band_bypass, false);
  std::ranges::fill(band_intensity, 1.0F);
  std::ranges::fill(band_last_L, 0.0F);
  std::ranges::fill(band_last_R, 0.0F);

  frequencies[0] = 20.0F;
  frequencies[1] = 520.0F;
  frequencies[2] = 1020.0F;
  frequencies[3] = 2020.0F;
  frequencies[4] = 3020.0F;
  frequencies[5] = 4020.0F;
  frequencies[6] = 5020.0F;
  frequencies[7] = 6020.0F;
  frequencies[8] = 7020.0F;
  frequencies[9] = 8020.0F;
  frequencies[10] = 9020.0F;
  frequencies[11] = 10020.0F;
  frequencies[12] = 15020.0F;
  frequencies[13] = 20020.0F;

  for (uint n = 0U; n < nbands; n++) {
    bind_band(static_cast<int>(n));
  }

  setup_input_output_gain();
}

Crystalizer::~Crystalizer() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  data_mutex.lock();

  filters_are_ready = false;

  data_mutex.unlock();

  util::debug(log_tag + name + " destroyed");
}

void Crystalizer::setup() {
  filters_are_ready = false;

  /*
    As zita uses fftw we have to be careful when reinitializing it. The thread that creates the fftw plan has to be the
    same that destroys it. Otherwise segmentation faults can happen. As we do not want to do this initializing in the
    plugin realtime thread we send it to the main thread through Glib::signal_idle().connect_once
  */

  Glib::signal_idle().connect_once([&, this] {
    blocksize = n_samples;

    n_samples_is_power_of_2 = (n_samples & (n_samples - 1)) == 0 && n_samples != 0;

    if (!n_samples_is_power_of_2) {
      while ((blocksize & (blocksize - 1)) != 0 && blocksize > 2) {
        blocksize--;
      }
    }

    util::debug(log_tag + name + " blocksize: " + std::to_string(blocksize));

    notify_latency = true;
    do_first_rotation = true;

    latency_n_frames = 1U;  // the second derivative forces us to delay at least one sample

    deque_out_L.resize(0);
    deque_out_R.resize(0);

    data_L.resize(0);
    data_R.resize(0);

    for (uint n = 0U; n < nbands; n++) {
      band_data_L.at(n).resize(blocksize);
      band_data_R.at(n).resize(blocksize);

      band_second_derivative_L.at(n).resize(blocksize);
      band_second_derivative_R.at(n).resize(blocksize);
    }

    for (uint n = 0U; n < nbands; n++) {
      filters.at(n)->set_n_samples(blocksize);
      filters.at(n)->set_rate(rate);

      filters.at(n)->set_min_frequency(frequencies.at(n));
      filters.at(n)->set_max_frequency(frequencies.at(n + 1U));

      filters.at(n)->setup();
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    filters_are_ready = true;
  });
}

void Crystalizer::process(std::span<float>& left_in,
                          std::span<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !filters_are_ready) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (n_samples_is_power_of_2) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    enhance_peaks(left_out, right_out);
  } else {
    for (size_t j = 0U, li_size = left_in.size(); j < li_size; j++) {
      data_L.emplace_back(left_in[j]);
      data_R.emplace_back(right_in[j]);

      if (data_L.size() == blocksize) {
        enhance_peaks(data_L, data_R);

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
      uint offset = 2U * (left_out.size() - deque_out_L.size());

      if (offset != latency_n_frames) {
        latency_n_frames = offset + 1U;  // the second derivative forces us to delay at least one sample

        notify_latency = true;
      }

      for (uint n = 0U, lo_size = left_out.size(); !deque_out_L.empty() && n < lo_size; n++) {
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

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    const float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

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

void Crystalizer::bind_band(const int& n) {
  const auto& bandn = "band" + std::to_string(n);

  band_intensity.at(n) = static_cast<float>(util::db_to_linear(settings->get_double("intensity-" + bandn)));
  band_mute.at(n) = settings->get_boolean("mute-" + bandn);
  band_bypass.at(n) = settings->get_boolean("bypass-" + bandn);

  settings->signal_changed("intensity-" + bandn).connect([=, this](const auto& key) {
    band_intensity.at(n) = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("mute-" + bandn).connect([=, this](const auto& key) {
    band_mute.at(n) = settings->get_boolean(key);
  });

  settings->signal_changed("bypass-" + bandn).connect([=, this](const auto& key) {
    band_bypass.at(n) = settings->get_boolean(key);
  });
}
