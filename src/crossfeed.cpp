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

#include "crossfeed.hpp"

Crossfeed::Crossfeed(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_path,
                     PipeManager* pipe_manager)
    : PluginBase(tag, tags::plugin_name::crossfeed, tags::plugin_package::bs2b, schema, schema_path, pipe_manager) {
  bs2b.set_level_fcut(g_settings_get_int(settings, "fcut"));

  bs2b.set_level_feed(10 * static_cast<int>(g_settings_get_double(settings, "feed")));

  gconnections.push_back(g_signal_connect(settings, "changed::fcut",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Crossfeed*>(user_data);

                                            std::scoped_lock<std::mutex> lock(self->data_mutex);

                                            self->bs2b.set_level_fcut(g_settings_get_int(settings, key));
                                          }),
                                          this));

  gconnections.push_back(
      g_signal_connect(settings, "changed::feed", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                         auto self = static_cast<Crossfeed*>(user_data);

                         std::scoped_lock<std::mutex> lock(self->data_mutex);

                         self->bs2b.set_level_feed(10 * static_cast<int>(g_settings_get_double(settings, key)));
                       }),
                       this));

  setup_input_output_gain();
}

Crossfeed::~Crossfeed() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Crossfeed::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  data.resize(2U * static_cast<size_t>(n_samples));

  if (rate != bs2b.get_srate()) {
    bs2b.set_srate(rate);
  }
}

void Crossfeed::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
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

  bs2b.cross_feed(data.data(), static_cast<int>(n_samples));

  for (size_t n = 0U; n < left_out.size(); n++) {
    left_out[n] = data[n * 2U];
    right_out[n] = data[n * 2U + 1U];
  }

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

auto Crossfeed::get_latency_seconds() -> float {
  return 0.0F;
}
