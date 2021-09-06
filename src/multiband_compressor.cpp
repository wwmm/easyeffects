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

#include "multiband_compressor.hpp"

MultibandCompressor::MultibandCompressor(const std::string& tag,
                                         const std::string& schema,
                                         const std::string& schema_path,
                                         PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::multiband_compressor, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/mb_compressor_stereo")) {
  if (!lv2_wrapper->found_plugin) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/mb_compressor_stereo is not installed");
  }

  lv2_wrapper->bind_key_enum(settings, "compressor-mode", "mode");

  lv2_wrapper->bind_key_enum(settings, "envelope-boost", "envb");

  for (uint n = 0U; n < n_bands; n++) {
    const auto& nstr = std::to_string(n);

    if (n > 0U) {
      lv2_wrapper->bind_key_bool(settings, "enable-band" + nstr, "cbe_" + nstr);

      lv2_wrapper->bind_key_double(settings, "split-frequency" + nstr, "sf_" + nstr);
    }

    lv2_wrapper->bind_key_enum(settings, "sidechain-source" + nstr, "scs_" + nstr);

    lv2_wrapper->bind_key_enum(settings, "sidechain-mode" + nstr, "scm_" + nstr);

    lv2_wrapper->bind_key_double(settings, "sidechain-lookahead" + nstr, "sla_" + nstr);

    lv2_wrapper->bind_key_double(settings, "sidechain-reactivity" + nstr, "scr_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp" + nstr, "scp_" + nstr);

    lv2_wrapper->bind_key_bool(settings, "sidechain-custom-lowcut-filter" + nstr, "sclc_" + nstr);

    lv2_wrapper->bind_key_bool(settings, "sidechain-custom-highcut-filter" + nstr, "schc_" + nstr);

    lv2_wrapper->bind_key_double(settings, "sidechain-lowcut-frequency" + nstr, "sclf_" + nstr);

    lv2_wrapper->bind_key_double(settings, "sidechain-highcut-frequency" + nstr, "schf_" + nstr);

    lv2_wrapper->bind_key_enum(settings, "compression-mode" + nstr, "cm_" + nstr);

    lv2_wrapper->bind_key_bool(settings, "compressor-enable" + nstr, "ce_" + nstr);

    lv2_wrapper->bind_key_bool(settings, "solo" + nstr, "bs_" + nstr);

    lv2_wrapper->bind_key_bool(settings, "mute" + nstr, "bm_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "attack-threshold" + nstr, "al_" + nstr);

    lv2_wrapper->bind_key_double(settings, "attack-time" + nstr, "at_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "release-threshold" + nstr, "rrl_" + nstr);

    lv2_wrapper->bind_key_double(settings, "release-time" + nstr, "rt_" + nstr);

    lv2_wrapper->bind_key_double(settings, "ratio" + nstr, "cr_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "knee" + nstr, "kn_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "boost-threshold" + nstr, "bth_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "boost-amount" + nstr, "bsa_" + nstr);

    lv2_wrapper->bind_key_double_db(settings, "makeup" + nstr, "mk_" + nstr);
  }

  setup_input_output_gain();
}

MultibandCompressor::~MultibandCompressor() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
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

  /*
   This plugin gives the latency in number of samples
 */

  const auto& lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    const float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + std::to_string(latency_value) + " s");

    Glib::signal_idle().connect_once([=, this] { latency.emit(latency_value); });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_value * 1000000000.0F);

    std::array<char, 1024> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      std::array<float, n_bands> frequency_range_end_array{};
      std::array<float, n_bands> envelope_array{};
      std::array<float, n_bands> curve_array{};
      std::array<float, n_bands> reduction_array{};

      for (uint n = 0U; n < n_bands; n++) {
        const auto& nstr = std::to_string(n);

        frequency_range_end_array.at(n) = lv2_wrapper->get_control_port_value("fre_" + nstr);
        envelope_array.at(n) = lv2_wrapper->get_control_port_value("elm_" + nstr);
        curve_array.at(n) = lv2_wrapper->get_control_port_value("clm_" + nstr);
        reduction_array.at(n) = lv2_wrapper->get_control_port_value("rlm_" + nstr);
      }

      Glib::signal_idle().connect_once([=, this] {
        frequency_range.emit(frequency_range_end_array);
        envelope.emit(envelope_array);
        curve.emit(curve_array);
        reduction.emit(reduction_array);
      });

      notify();

      notification_dt = 0.0F;
    }
  }
}
