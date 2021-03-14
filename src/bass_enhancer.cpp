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

#include "bass_enhancer.hpp"

BassEnhancer::BassEnhancer(const std::string& tag,
                           const std::string& schema,
                           const std::string& schema_path,
                           PipeManager* pipe_manager)
    : PluginBase(tag, "bass_enhancer", schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/BassEnhancer")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_control_port_value("amount", static_cast<float>(util::db_to_linear(settings->get_double("amount"))));

  lv2_wrapper->set_control_port_value("harmonics", static_cast<float>(settings->get_double("harmonics")));

  lv2_wrapper->set_control_port_value("scope", static_cast<float>(settings->get_double("scope")));

  lv2_wrapper->set_control_port_value("floor", static_cast<float>(settings->get_double("floor")));

  lv2_wrapper->set_control_port_value("blend", static_cast<float>(settings->get_double("blend")));

  lv2_wrapper->set_control_port_value("floor-active", static_cast<float>(settings->get_boolean("floor-active")));

  lv2_wrapper->set_control_port_value("listen", static_cast<float>(settings->get_boolean("listen")));

  // signal connection

  settings->signal_changed("amount").connect([=, this](auto key) {
    lv2_wrapper->set_control_port_value(key, static_cast<float>(util::db_to_linear(settings->get_double(key))));
  });

  settings->signal_changed("harmonics").connect([=, this](auto key) {
    lv2_wrapper->set_control_port_value(key, settings->get_double(key));
  });

  settings->signal_changed("scope").connect(
      [=, this](auto key) { lv2_wrapper->set_control_port_value(key, settings->get_double(key)); });

  settings->signal_changed("floor").connect(
      [=, this](auto key) { lv2_wrapper->set_control_port_value(key, settings->get_double(key)); });

  settings->signal_changed("blend").connect(
      [=, this](auto key) { lv2_wrapper->set_control_port_value(key, settings->get_double(key)); });

  settings->signal_changed("floor-active").connect([=, this](auto key) {
    lv2_wrapper->set_control_port_value(key, settings->get_boolean(key));
  });

  settings->signal_changed("listen").connect(
      [=, this](auto key) { lv2_wrapper->set_control_port_value(key, settings->get_boolean(key)); });
}

BassEnhancer::~BassEnhancer() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void BassEnhancer::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  if (!lv2_wrapper->create_instance(rate)) {
    bypass = true;
  } else {
    lv2_wrapper->set_control_port_value("bypass", 0.0F);
  }
}

void BassEnhancer::process(std::vector<float>& left_in,
                           std::vector<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (lv2_wrapper->get_n_samples() != left_in.size()) {
    lv2_wrapper->deactivate();

    lv2_wrapper->set_n_samples(left_in.size());

    lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);

    lv2_wrapper->activate();
  }

  lv2_wrapper->run();

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

void BassEnhancer::bind_to_gsettings() {
  // g_settings_bind_with_mapping(settings, "input-gain", bass_enhancer, "level-in", G_SETTINGS_BIND_DEFAULT,
  //                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  // g_settings_bind_with_mapping(settings, "output-gain", bass_enhancer, "level-out", G_SETTINGS_BIND_DEFAULT,
  //                              util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);
}
