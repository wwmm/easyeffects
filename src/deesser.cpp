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

#include "deesser.hpp"

Deesser::Deesser(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::deesser, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/Deesser")) {
  if (!lv2_wrapper->found_plugin) {
    util::warning(log_tag + "http://calf.sourceforge.net/plugins/Deesser is not installed");
  }

  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "mode", "mode");

  lv2_wrapper->bind_key_enum(settings, "detection", "detection");

  lv2_wrapper->bind_key_double(settings, "ratio", "ratio");

  lv2_wrapper->bind_key_double(settings, "f1-freq", "f1_freq");

  lv2_wrapper->bind_key_double(settings, "f2-freq", "f2_freq");

  lv2_wrapper->bind_key_double(settings, "f2-q", "f2_q");

  lv2_wrapper->bind_key_double_db(settings, "threshold", "threshold");

  lv2_wrapper->bind_key_double_db(settings, "makeup", "makeup");

  lv2_wrapper->bind_key_double_db(settings, "f1-level", "f1_level");

  lv2_wrapper->bind_key_double_db(settings, "f2-level", "f2_level");

  lv2_wrapper->bind_key_int(settings, "laxity", "laxity");

  lv2_wrapper->bind_key_bool(settings, "sc-listen", "sc_listen");
}

Deesser::~Deesser() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Deesser::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void Deesser::process(std::span<float>& left_in,
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
      float detected_value = lv2_wrapper->get_control_port_value("detected");
      float compression_value = lv2_wrapper->get_control_port_value("compression");

      Glib::signal_idle().connect_once([=, this] {
        detected.emit(detected_value);
        compression.emit(compression_value);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}
