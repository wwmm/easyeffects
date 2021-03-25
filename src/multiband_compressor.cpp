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

#include "multiband_compressor.hpp"

MultibandCompressor::MultibandCompressor(const std::string& tag,
                                         const std::string& schema,
                                         const std::string& schema_path,
                                         PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::multiband_compressor, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/MultibandCompressor")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });
}

MultibandCompressor::~MultibandCompressor() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void MultibandCompressor::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void MultibandCompressor::process(std::span<float>& left_in,
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

      float compression0_value = lv2_wrapper->get_control_port_value("compression0");
      float compression1_value = lv2_wrapper->get_control_port_value("compression1");
      float compression2_value = lv2_wrapper->get_control_port_value("compression2");
      float compression3_value = lv2_wrapper->get_control_port_value("compression3");

      Glib::signal_idle().connect_once([=, this] {
        output0.emit(output0_value);
        output1.emit(output1_value);
        output2.emit(output2_value);
        output3.emit(output3_value);

        compression0.emit(compression0_value);
        compression1.emit(compression1_value);
        compression2.emit(compression2_value);
        compression3.emit(compression3_value);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}

// g_settings_bind_with_mapping(settings, "freq0", multiband_compressor, "freq0", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "freq1", multiband_compressor, "freq1", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "freq2", multiband_compressor, "freq2", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind(settings, "mode", multiband_compressor, "mode", G_SETTINGS_BIND_DEFAULT);

// // sub band

// g_settings_bind_with_mapping(settings, "threshold0", multiband_compressor, "threshold0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio0", multiband_compressor, "ratio0", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack0", multiband_compressor, "attack0", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release0", multiband_compressor, "release0", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup0", multiband_compressor, "makeup0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee0", multiband_compressor, "knee0", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection0", multiband_compressor, "detection0", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass0", multiband_compressor, "bypass0", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo0", multiband_compressor, "solo0", G_SETTINGS_BIND_DEFAULT);

// // low band

// g_settings_bind_with_mapping(settings, "threshold1", multiband_compressor, "threshold1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio1", multiband_compressor, "ratio1", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack1", multiband_compressor, "attack1", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release1", multiband_compressor, "release1", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup1", multiband_compressor, "makeup1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee1", multiband_compressor, "knee1", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection1", multiband_compressor, "detection1", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass1", multiband_compressor, "bypass1", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo1", multiband_compressor, "solo1", G_SETTINGS_BIND_DEFAULT);

// // mid band

// g_settings_bind_with_mapping(settings, "threshold2", multiband_compressor, "threshold2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio2", multiband_compressor, "ratio2", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack2", multiband_compressor, "attack2", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release2", multiband_compressor, "release2", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup2", multiband_compressor, "makeup2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee2", multiband_compressor, "knee2", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection2", multiband_compressor, "detection2", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass2", multiband_compressor, "bypass2", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo2", multiband_compressor, "solo2", G_SETTINGS_BIND_DEFAULT);

// // high band

// g_settings_bind_with_mapping(settings, "threshold3", multiband_compressor, "threshold3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "ratio3", multiband_compressor, "ratio3", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "attack3", multiband_compressor, "attack3", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "release3", multiband_compressor, "release3", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "makeup3", multiband_compressor, "makeup3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "knee3", multiband_compressor, "knee3", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind(settings, "detection3", multiband_compressor, "detection3", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "bypass3", multiband_compressor, "bypass3", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "solo3", multiband_compressor, "solo3", G_SETTINGS_BIND_DEFAULT);
