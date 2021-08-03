/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#include "gate.hpp"

Gate::Gate(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::gate, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/Gate")) {
  if (!lv2_wrapper->found_plugin) {
    util::warning(log_tag + "http://calf.sourceforge.net/plugins/Gate is not installed");
  }

  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "detection", "detection");

  lv2_wrapper->bind_key_enum(settings, "stereo-link", "stereo_link");

  lv2_wrapper->bind_key_double(settings, "attack", "attack");

  lv2_wrapper->bind_key_double(settings, "release", "release");

  lv2_wrapper->bind_key_double(settings, "ratio", "ratio");

  lv2_wrapper->bind_key_double_db(settings, "range", "range");

  lv2_wrapper->bind_key_double_db(settings, "threshold", "threshold");

  lv2_wrapper->bind_key_double_db(settings, "knee", "knee");

  lv2_wrapper->bind_key_double_db(settings, "makeup", "makeup");
}

Gate::~Gate() {
  util::debug(log_tag + name + " destroyed");

  if (connected_to_pw) {
    disconnect_from_pw();
  }
}

void Gate::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void Gate::process(std::span<float>& left_in,
                   std::span<float>& right_in,
                   std::span<float>& left_out,
                   std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      float gating_value = lv2_wrapper->get_control_port_value("gating");

      Glib::signal_idle().connect_once([=, this] { gating.emit(gating_value); });

      notify();

      notification_dt = 0.0F;
    }
  }
}
