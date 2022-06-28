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

#include "multiband_gate.hpp"

MultibandGate::MultibandGate(const std::string& tag,
                             const std::string& schema,
                             const std::string& schema_path,
                             PipeManager* pipe_manager)
    : PluginBase(tag, tags::plugin_name::multiband_gate, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/MultibandGate")) {
  if (!lv2_wrapper->found_plugin) {
    util::debug(log_tag + "http://calf.sourceforge.net/plugins/MultibandGate is not installed");
  }

  lv2_wrapper->bind_key_enum<"mode", "mode">(settings);

  lv2_wrapper->bind_key_double<"freq0", "freq0">(settings);

  lv2_wrapper->bind_key_double<"freq1", "freq1">(settings);

  lv2_wrapper->bind_key_double<"freq2", "freq2">(settings);

  // sub band

  lv2_wrapper->bind_key_double_db<"range0", "range0">(settings);

  lv2_wrapper->bind_key_double_db<"threshold0", "threshold0">(settings);

  lv2_wrapper->bind_key_double_db<"makeup0", "makeup0">(settings);

  lv2_wrapper->bind_key_double_db<"knee0", "knee0">(settings);

  lv2_wrapper->bind_key_double<"ratio0", "ratio0">(settings);

  lv2_wrapper->bind_key_double<"attack0", "attack0">(settings);

  lv2_wrapper->bind_key_double<"release0", "release0">(settings);

  lv2_wrapper->bind_key_enum<"detection0", "detection0">(settings);

  lv2_wrapper->bind_key_bool<"bypass0", "bypass0">(settings);

  lv2_wrapper->bind_key_bool<"solo0", "solo0">(settings);

  // low band

  lv2_wrapper->bind_key_double_db<"range1", "range1">(settings);

  lv2_wrapper->bind_key_double_db<"threshold1", "threshold1">(settings);

  lv2_wrapper->bind_key_double_db<"makeup1", "makeup1">(settings);

  lv2_wrapper->bind_key_double_db<"knee1", "knee1">(settings);

  lv2_wrapper->bind_key_double<"ratio1", "ratio1">(settings);

  lv2_wrapper->bind_key_double<"attack1", "attack1">(settings);

  lv2_wrapper->bind_key_double<"release1", "release1">(settings);

  lv2_wrapper->bind_key_enum<"detection1", "detection1">(settings);

  lv2_wrapper->bind_key_bool<"bypass1", "bypass1">(settings);

  lv2_wrapper->bind_key_bool<"solo1", "solo1">(settings);

  // mid band

  lv2_wrapper->bind_key_double_db<"range2", "range2">(settings);

  lv2_wrapper->bind_key_double_db<"threshold2", "threshold2">(settings);

  lv2_wrapper->bind_key_double_db<"makeup2", "makeup2">(settings);

  lv2_wrapper->bind_key_double_db<"knee2", "knee2">(settings);

  lv2_wrapper->bind_key_double<"ratio2", "ratio2">(settings);

  lv2_wrapper->bind_key_double<"attack2", "attack2">(settings);

  lv2_wrapper->bind_key_double<"release2", "release2">(settings);

  lv2_wrapper->bind_key_enum<"detection2", "detection2">(settings);

  lv2_wrapper->bind_key_bool<"bypass2", "bypass2">(settings);

  lv2_wrapper->bind_key_bool<"solo2", "solo2">(settings);

  // high band

  lv2_wrapper->bind_key_double_db<"range3", "range3">(settings);

  lv2_wrapper->bind_key_double_db<"threshold3", "threshold3">(settings);

  lv2_wrapper->bind_key_double_db<"makeup3", "makeup3">(settings);

  lv2_wrapper->bind_key_double_db<"knee3", "knee3">(settings);

  lv2_wrapper->bind_key_double<"ratio3", "ratio3">(settings);

  lv2_wrapper->bind_key_double<"attack3", "attack3">(settings);

  lv2_wrapper->bind_key_double<"release3", "release3">(settings);

  lv2_wrapper->bind_key_enum<"detection3", "detection3">(settings);

  lv2_wrapper->bind_key_bool<"bypass3", "bypass3">(settings);

  lv2_wrapper->bind_key_bool<"solo3", "solo3">(settings);

  setup_input_output_gain();
}

MultibandGate::~MultibandGate() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void MultibandGate::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void MultibandGate::process(std::span<float>& left_in,
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

    notification_dt += buffer_duration;

    if (notification_dt >= notification_time_window) {
      // values needed as double for levelbars widget ui, so we convert them here

      output0_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("output0"));
      output1_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("output1"));
      output2_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("output2"));
      output3_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("output3"));

      gating0_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("gating0"));
      gating1_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("gating1"));
      gating2_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("gating2"));
      gating3_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("gating3"));

      g_idle_add((GSourceFunc) +
                     [](gpointer user_data) {
                       auto* self = static_cast<MultibandGate*>(user_data);

                       if (!self->post_messages) {
                         return G_SOURCE_REMOVE;
                       }

                       if (self->output0.empty() || self->output1.empty() || self->output2.empty() ||
                           self->output3.empty() || self->gating0.empty() || self->gating1.empty() ||
                           self->gating2.empty() || self->gating3.empty()) {
                         return G_SOURCE_REMOVE;
                       }

                       self->output0.emit(self->output0_port_value);
                       self->output1.emit(self->output1_port_value);
                       self->output2.emit(self->output2_port_value);
                       self->output3.emit(self->output3_port_value);

                       self->gating0.emit(self->gating0_port_value);
                       self->gating1.emit(self->gating1_port_value);
                       self->gating2.emit(self->gating2_port_value);
                       self->gating3.emit(self->gating3_port_value);

                       return G_SOURCE_REMOVE;
                     },
                 this);

      notify();

      notification_dt = 0.0F;
    }
  }
}

auto MultibandGate::get_latency_seconds() -> float {
  return 0.0F;
}