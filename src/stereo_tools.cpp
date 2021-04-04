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

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_double(settings, "balance-in", "balance_in");

  lv2_wrapper->bind_key_double(settings, "balance-out", "balance_out");

  lv2_wrapper->bind_key_double(settings, "sbal", "sbal");

  lv2_wrapper->bind_key_double(settings, "mpan", "mpan");

  lv2_wrapper->bind_key_double(settings, "stereo-base", "stereo_base");

  lv2_wrapper->bind_key_double(settings, "delay", "delay");

  lv2_wrapper->bind_key_double(settings, "sc-level", "sc_level");

  lv2_wrapper->bind_key_double(settings, "stereo-phase", "stereo_phase");

  lv2_wrapper->bind_key_double_db(settings, "slev", "slev");

  lv2_wrapper->bind_key_double_db(settings, "mlev", "mlev");

  lv2_wrapper->bind_key_bool(settings, "softclip", "softclip");

  lv2_wrapper->bind_key_bool(settings, "mutel", "mutel");

  lv2_wrapper->bind_key_bool(settings, "muter", "muter");

  lv2_wrapper->bind_key_bool(settings, "phasel", "phasel");

  lv2_wrapper->bind_key_bool(settings, "phaser", "phaser");

  lv2_wrapper->bind_key_enum(settings, "mode", "mode");

  initialize_listener();
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

  lv2_wrapper->set_n_samples(n_samples);
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

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      float correlation = lv2_wrapper->get_control_port_value("meter_phase");

      Glib::signal_idle().connect_once([=, this] { new_correlation.emit(correlation); });

      notify();

      notification_dt = 0.0F;
    }
  }
}
