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

#include "multiband_gate.hpp"

MultibandGate::MultibandGate(const std::string& tag,
                             const std::string& schema,
                             const std::string& schema_path,
                             PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::multiband_gate, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/MultibandGate")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "mode", "mode");

  lv2_wrapper->bind_key_double(settings, "freq0", "freq0");

  lv2_wrapper->bind_key_double(settings, "freq1", "freq1");

  lv2_wrapper->bind_key_double(settings, "freq2", "freq2");

  // sub band

  lv2_wrapper->bind_key_double_db(settings, "range0", "range0");

  lv2_wrapper->bind_key_double_db(settings, "threshold0", "threshold0");

  lv2_wrapper->bind_key_double_db(settings, "makeup0", "makeup0");

  lv2_wrapper->bind_key_double_db(settings, "knee0", "knee0");

  lv2_wrapper->bind_key_double(settings, "ratio0", "ratio0");

  lv2_wrapper->bind_key_double(settings, "attack0", "attack0");

  lv2_wrapper->bind_key_double(settings, "release0", "release0");

  lv2_wrapper->bind_key_enum(settings, "detection0", "detection0");

  lv2_wrapper->bind_key_bool(settings, "bypass0", "bypass0");

  lv2_wrapper->bind_key_bool(settings, "solo0", "solo0");

  // low band

  lv2_wrapper->bind_key_double_db(settings, "range1", "range1");

  lv2_wrapper->bind_key_double_db(settings, "threshold1", "threshold1");

  lv2_wrapper->bind_key_double_db(settings, "makeup1", "makeup1");

  lv2_wrapper->bind_key_double_db(settings, "knee1", "knee1");

  lv2_wrapper->bind_key_double(settings, "ratio1", "ratio1");

  lv2_wrapper->bind_key_double(settings, "attack1", "attack1");

  lv2_wrapper->bind_key_double(settings, "release1", "release1");

  lv2_wrapper->bind_key_enum(settings, "detection1", "detection1");

  lv2_wrapper->bind_key_bool(settings, "bypass1", "bypass1");

  lv2_wrapper->bind_key_bool(settings, "solo1", "solo1");

  // mid band

  lv2_wrapper->bind_key_double_db(settings, "range2", "range2");

  lv2_wrapper->bind_key_double_db(settings, "threshold2", "threshold2");

  lv2_wrapper->bind_key_double_db(settings, "makeup2", "makeup2");

  lv2_wrapper->bind_key_double_db(settings, "knee2", "knee2");

  lv2_wrapper->bind_key_double(settings, "ratio2", "ratio2");

  lv2_wrapper->bind_key_double(settings, "attack2", "attack2");

  lv2_wrapper->bind_key_double(settings, "release2", "release2");

  lv2_wrapper->bind_key_enum(settings, "detection2", "detection2");

  lv2_wrapper->bind_key_bool(settings, "bypass2", "bypass2");

  lv2_wrapper->bind_key_bool(settings, "solo2", "solo2");

  // high band

  lv2_wrapper->bind_key_double_db(settings, "range3", "range3");

  lv2_wrapper->bind_key_double_db(settings, "threshold3", "threshold3");

  lv2_wrapper->bind_key_double_db(settings, "makeup3", "makeup3");

  lv2_wrapper->bind_key_double_db(settings, "knee3", "knee3");

  lv2_wrapper->bind_key_double(settings, "ratio3", "ratio3");

  lv2_wrapper->bind_key_double(settings, "attack3", "attack3");

  lv2_wrapper->bind_key_double(settings, "release3", "release3");

  lv2_wrapper->bind_key_enum(settings, "detection3", "detection3");

  lv2_wrapper->bind_key_bool(settings, "bypass3", "bypass3");

  lv2_wrapper->bind_key_bool(settings, "solo3", "solo3");
}

MultibandGate::~MultibandGate() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void MultibandGate::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
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

  apply_gain(left_in, right_in, input_gain);

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      float output0_value = lv2_wrapper->get_control_port_value("output0");
      float output1_value = lv2_wrapper->get_control_port_value("output1");
      float output2_value = lv2_wrapper->get_control_port_value("output2");
      float output3_value = lv2_wrapper->get_control_port_value("output3");

      float gating0_value = lv2_wrapper->get_control_port_value("gating0");
      float gating1_value = lv2_wrapper->get_control_port_value("gating1");
      float gating2_value = lv2_wrapper->get_control_port_value("gating2");
      float gating3_value = lv2_wrapper->get_control_port_value("gating3");

      Glib::signal_idle().connect_once([=, this] {
        output0.emit(output0_value);
        output1.emit(output1_value);
        output2.emit(output2_value);
        output3.emit(output3_value);

        gating0.emit(gating0_value);
        gating1.emit(gating1_value);
        gating2.emit(gating2_value);
        gating3.emit(gating3_value);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}

// g_settings_bind_with_mapping(settings, "freq0", multiband_gate, "freq0", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "freq1", multiband_gate, "freq1", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "freq2", multiband_gate, "freq2", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind(settings, "mode", multiband_gate, "mode", G_SETTINGS_BIND_DEFAULT);

// // sub band

// g_settings_bind_with_mapping(settings, "range0", multiband_gate, "range0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "threshold0", multiband_gate, "threshold0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio0", multiband_gate, "ratio0", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack0", multiband_gate, "attack0", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release0", multiband_gate, "release0", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup0", multiband_gate, "makeup0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee0", multiband_gate, "knee0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection0", multiband_gate, "detection0", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass0", multiband_gate, "bypass0", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo0", multiband_gate, "solo0", G_SETTINGS_BIND_DEFAULT);

// // low band

// g_settings_bind_with_mapping(settings, "range1", multiband_gate, "range1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "threshold1", multiband_gate, "threshold1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio1", multiband_gate, "ratio1", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack1", multiband_gate, "attack1", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release1", multiband_gate, "release1", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup1", multiband_gate, "makeup1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee1", multiband_gate, "knee1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection1", multiband_gate, "detection1", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass1", multiband_gate, "bypass1", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo1", multiband_gate, "solo1", G_SETTINGS_BIND_DEFAULT);

// // mid

// g_settings_bind_with_mapping(settings, "range2", multiband_gate, "range2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "threshold2", multiband_gate, "threshold2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio2", multiband_gate, "ratio2", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack2", multiband_gate, "attack2", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release2", multiband_gate, "release2", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup2", multiband_gate, "makeup2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee2", multiband_gate, "knee2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection2", multiband_gate, "detection2", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass2", multiband_gate, "bypass2", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo2", multiband_gate, "solo2", G_SETTINGS_BIND_DEFAULT);

// // high band

// g_settings_bind_with_mapping(settings, "range3", multiband_gate, "range3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "threshold3", multiband_gate, "threshold3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio3", multiband_gate, "ratio3", G_SETTINGS_BIND_GET,
// util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack3", multiband_gate, "attack3", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release3", multiband_gate, "release3", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup3", multiband_gate, "makeup3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee3", multiband_gate, "knee3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection3", multiband_gate, "detection3", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass3", multiband_gate, "bypass3", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo3", multiband_gate, "solo3", G_SETTINGS_BIND_DEFAULT);
