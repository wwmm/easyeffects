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

#include "multiband_compressor.hpp"

MultibandCompressor::MultibandCompressor(const std::string& tag,
                                         const std::string& schema,
                                         const std::string& schema_path,
                                         PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::multiband_compressor, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/mb_compressor_stereo")) {
  if (!lv2_wrapper->found_plugin) {
    util::warning(log_tag + "http://lsp-plug.in/plugins/lv2/mb_compressor_stereo is not installed");
  }

  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "compressor-mode", "mode");

  lv2_wrapper->bind_key_enum(settings, "envelope-boost", "envb");

  for (uint n = 0; n < n_bands; n++) {
    if (n > 0) {
      lv2_wrapper->bind_key_bool(settings, "enable-band" + std::to_string(n), "cbe_" + std::to_string(n));

      lv2_wrapper->bind_key_double(settings, "split-frequency" + std::to_string(n), "sf_" + std::to_string(n));
    }

    lv2_wrapper->bind_key_enum(settings, "sidechain-source" + std::to_string(n), "scs_" + std::to_string(n));

    lv2_wrapper->bind_key_enum(settings, "sidechain-mode" + std::to_string(n), "scm_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "sidechain-lookahead" + std::to_string(n), "sla_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "sidechain-reactivity" + std::to_string(n), "scr_" + std::to_string(n));

    lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp" + std::to_string(n), "scp_" + std::to_string(n));

    lv2_wrapper->bind_key_bool(settings, "sidechain-custom-lowcut-filter" + std::to_string(n),
                               "sclc_" + std::to_string(n));

    lv2_wrapper->bind_key_bool(settings, "sidechain-custom-highcut-filter" + std::to_string(n),
                               "schc_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "sidechain-lowcut-frequency" + std::to_string(n),
                                 "sclf_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "sidechain-highcut-frequency" + std::to_string(n),
                                 "schf_" + std::to_string(n));

    lv2_wrapper->bind_key_enum(settings, "compression-mode" + std::to_string(n), "cm_" + std::to_string(n));

    lv2_wrapper->bind_key_bool(settings, "solo" + std::to_string(n), "bs_" + std::to_string(n));

    lv2_wrapper->bind_key_bool(settings, "mute" + std::to_string(n), "bm_" + std::to_string(n));

    lv2_wrapper->bind_key_double_db(settings, "attack-threshold" + std::to_string(n), "al_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "attack-time" + std::to_string(n), "at_" + std::to_string(n));

    lv2_wrapper->bind_key_double_db(settings, "release-threshold" + std::to_string(n), "rrl_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "release-time" + std::to_string(n), "rt_" + std::to_string(n));

    lv2_wrapper->bind_key_double(settings, "ratio" + std::to_string(n), "cr_" + std::to_string(n));

    lv2_wrapper->bind_key_double_db(settings, "knee" + std::to_string(n), "kn_" + std::to_string(n));
  }

  initialize_listener();
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
      // float output0_value = lv2_wrapper->get_control_port_value("output0");
      // float output1_value = lv2_wrapper->get_control_port_value("output1");
      // float output2_value = lv2_wrapper->get_control_port_value("output2");
      // float output3_value = lv2_wrapper->get_control_port_value("output3");

      // float compression0_value = lv2_wrapper->get_control_port_value("compression0");
      // float compression1_value = lv2_wrapper->get_control_port_value("compression1");
      // float compression2_value = lv2_wrapper->get_control_port_value("compression2");
      // float compression3_value = lv2_wrapper->get_control_port_value("compression3");

      Glib::signal_idle().connect_once([=, this] {
        // output0.emit(output0_value);
        // output1.emit(output1_value);
        // output2.emit(output2_value);
        // output3.emit(output3_value);

        // compression0.emit(compression0_value);
        // compression1.emit(compression1_value);
        // compression2.emit(compression2_value);
        // compression3.emit(compression3_value);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}
