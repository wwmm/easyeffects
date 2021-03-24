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

#include "compressor.hpp"

Compressor::Compressor(const std::string& tag,
                       const std::string& schema,
                       const std::string& schema_path,
                       PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::compressor, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/compressor_stereo")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "mode", "cm");

  lv2_wrapper->bind_key_enum(settings, "sidechain-type", "sct");

  lv2_wrapper->bind_key_enum(settings, "sidechain-mode", "scm");

  lv2_wrapper->bind_key_enum(settings, "sidechain-source", "scs");

  lv2_wrapper->bind_key_enum(settings, "hpf-mode", "shpm");

  lv2_wrapper->bind_key_enum(settings, "lpf-mode", "slpm");

  lv2_wrapper->bind_key_bool(settings, "sidechain-listen", "scl");

  lv2_wrapper->bind_key_double(settings, "attack", "at");

  lv2_wrapper->bind_key_double(settings, "release", "rt");

  lv2_wrapper->bind_key_double(settings, "ratio", "cr");

  lv2_wrapper->bind_key_double(settings, "sidechain-reactivity", "scr");

  lv2_wrapper->bind_key_double(settings, "sidechain-lookahead", "sla");

  lv2_wrapper->bind_key_double(settings, "hpf-frequency", "shpf");

  lv2_wrapper->bind_key_double(settings, "lpf-frequency", "slpf");

  lv2_wrapper->bind_key_double_db(settings, "release-threshold", "rrl");

  lv2_wrapper->bind_key_double_db(settings, "boost-threshold", "bth");

  lv2_wrapper->bind_key_double_db(settings, "threshold", "al");

  lv2_wrapper->bind_key_double_db(settings, "knee", "kn");

  lv2_wrapper->bind_key_double_db(settings, "makeup", "mk");

  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp", "scp");
}

Compressor::~Compressor() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void Compressor::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void Compressor::process(std::span<float>& left_in,
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
      float reduction_value = lv2_wrapper->get_control_port_value("rlm");
      float sidechain_value = lv2_wrapper->get_control_port_value("slm");
      float curve_value = lv2_wrapper->get_control_port_value("clm");
      float latency_value = lv2_wrapper->get_control_port_value("out_latency");

      Glib::signal_idle().connect_once([=, this] {
        reduction.emit(reduction_value);
        sidechain.emit(sidechain_value);
        curve.emit(curve_value);
        latency.emit(latency_value);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}
