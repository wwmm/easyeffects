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

#include "bass_enhancer.hpp"

BassEnhancer::BassEnhancer(const std::string& tag,
                           const std::string& schema,
                           const std::string& schema_path,
                           PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::bass_enhancer, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/BassEnhancer")) {
  if (!lv2_wrapper->found_plugin) {
    util::debug(log_tag + "http://calf.sourceforge.net/plugins/BassEnhancer is not installed");
  }

  lv2_wrapper->bind_key_double_db(settings, "amount", "amount");

  lv2_wrapper->bind_key_double(settings, "harmonics", "drive");

  lv2_wrapper->bind_key_double(settings, "scope", "freq");

  lv2_wrapper->bind_key_double(settings, "floor", "floor");

  lv2_wrapper->bind_key_double(settings, "blend", "blend");

  lv2_wrapper->bind_key_bool(settings, "floor-active", "floor_active");

  lv2_wrapper->bind_key_bool(settings, "listen", "listen");

  setup_input_output_gain();
}

BassEnhancer::~BassEnhancer() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void BassEnhancer::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void BassEnhancer::process(std::span<float>& left_in,
                           std::span<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      // harmonics needed as double for levelbar widget ui, so we convert it here

      const double& harmonics_value = static_cast<double>(lv2_wrapper->get_control_port_value("meter_drive"));

      Glib::signal_idle().connect_once([=, this] { harmonics.emit(harmonics_value); });

      notify();

      notification_dt = 0.0F;
    }
  }
}
