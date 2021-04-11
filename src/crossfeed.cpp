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

#include "crossfeed.hpp"

Crossfeed::Crossfeed(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_path,
                     PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::crossfeed, schema, schema_path, pipe_manager) {
  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("fcut").connect([=, this](auto key) {
    std::lock_guard<std::mutex> lock(data_mutex);

    bs2b.set_level_fcut(settings->get_int(key));
  });

  settings->signal_changed("feed").connect([=, this](auto key) {
    std::lock_guard<std::mutex> lock(data_mutex);

    bs2b.set_level_feed(10 * settings->get_double(key));
  });

  initialize_listener();
}

Crossfeed::~Crossfeed() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void Crossfeed::setup() {
  std::lock_guard<std::mutex> lock(data_mutex);

  bs2b.set_srate(rate);

  data.resize(2 * n_samples);
}

void Crossfeed::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::lock_guard<std::mutex> lock(data_mutex);

  if (bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  for (size_t n = 0; n < left_in.size(); n++) {
    data[n * 2] = left_in[n];
    data[n * 2 + 1] = right_in[n];
  }

  bs2b.cross_feed(data.data(), n_samples);

  for (size_t n = 0; n < left_out.size(); n++) {
    left_out[n] = data[n * 2];
    right_out[n] = data[n * 2 + 1];
  }

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}
