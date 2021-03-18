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

#include "stereo_tools.hpp"

StereoTools::StereoTools(const std::string& tag,
                         const std::string& schema,
                         const std::string& schema_path,
                         PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::stereo_tools, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/StereoTools")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("bypass").connect([=, this](auto key) { bypass = settings->get_boolean(key); });

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });
}

StereoTools::~StereoTools() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void StereoTools::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->create_instance(rate);
}

void StereoTools::process(std::span<float>& left_in,
                          std::span<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  if (lv2_wrapper->get_n_samples() != left_in.size()) {
    lv2_wrapper->set_n_samples(left_in.size());
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);

  lv2_wrapper->run();

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

// g_settings_bind_with_mapping(settings, "input-gain", stereo_tools, "level-in", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "output-gain", stereo_tools, "level-out", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "balance-in", stereo_tools, "balance-in", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "balance-out", stereo_tools, "balance-out", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind(settings, "softclip", stereo_tools, "softclip", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "mutel", stereo_tools, "mutel", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "muter", stereo_tools, "muter", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "phasel", stereo_tools, "phasel", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "phaser", stereo_tools, "phaser", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind(settings, "mode", stereo_tools, "mode", G_SETTINGS_BIND_DEFAULT);

// g_settings_bind_with_mapping(settings, "slev", stereo_tools, "slev", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "sbal", stereo_tools, "sbal", G_SETTINGS_BIND_GET, util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "mlev", stereo_tools, "mlev", G_SETTINGS_BIND_DEFAULT,
//                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "mpan", stereo_tools, "mpan", G_SETTINGS_BIND_GET, util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "stereo-base", stereo_tools, "stereo-base", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "delay", stereo_tools, "delay", G_SETTINGS_BIND_GET, util::double_to_float,
//                              nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "sc-level", stereo_tools, "sc-level", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);

// g_settings_bind_with_mapping(settings, "stereo-phase", stereo_tools, "stereo-phase", G_SETTINGS_BIND_GET,
//                              util::double_to_float, nullptr, nullptr, nullptr);
