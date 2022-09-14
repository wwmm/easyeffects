/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "gate.hpp"

Gate::Gate(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager)
    : PluginBase(tag, tags::plugin_name::gate, tags::plugin_package::lsp, schema, schema_path, pipe_manager, true),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/sc_gate_stereo")) {
  package_installed = lv2_wrapper->found_plugin;

  if (!package_installed) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/sc_gate_stereo is not installed");
  }

  gconnections.push_back(g_signal_connect(settings, "changed::sidechain-input",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Gate*>(user_data);

                                            self->update_sidechain_links(key);
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::sidechain-input-device",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Gate*>(user_data);

                                            self->update_sidechain_links(key);
                                          }),
                                          this));

  lv2_wrapper->bind_key_enum<"sci", "sidechain-input">(settings);

  lv2_wrapper->bind_key_enum<"scm", "sidechain-mode">(settings);

  lv2_wrapper->bind_key_enum<"scs", "sidechain-source">(settings);

  lv2_wrapper->bind_key_enum<"shpm", "hpf-mode">(settings);

  lv2_wrapper->bind_key_enum<"slpm", "lpf-mode">(settings);

  lv2_wrapper->bind_key_bool<"scl", "sidechain-listen">(settings);

  lv2_wrapper->bind_key_double<"at", "attack">(settings);

  lv2_wrapper->bind_key_double<"rt", "release">(settings);

  lv2_wrapper->bind_key_double<"scr", "sidechain-reactivity">(settings);

  lv2_wrapper->bind_key_double<"sla", "sidechain-lookahead">(settings);

  lv2_wrapper->bind_key_double<"shpf", "hpf-frequency">(settings);

  lv2_wrapper->bind_key_double<"slpf", "lpf-frequency">(settings);

  lv2_wrapper->bind_key_double_db<"gt", "curve-threshold">(settings);

  lv2_wrapper->bind_key_double_db<"gz", "curve-zone">(settings);

  lv2_wrapper->bind_key_bool<"gh", "hysteresis">(settings);

  lv2_wrapper->bind_key_double_db<"ht", "hysteresis-threshold">(settings);

  lv2_wrapper->bind_key_double_db<"hz", "hysteresis-zone">(settings);

  lv2_wrapper->bind_key_double_db<"gr", "reduction">(settings);

  lv2_wrapper->bind_key_double_db<"mk", "makeup">(settings);

  lv2_wrapper->bind_key_double_db<"scp", "sidechain-preamp">(settings);

  // The following controls can assume -inf
  lv2_wrapper->bind_key_double_db<"cdr", "dry", false>(settings);

  lv2_wrapper->bind_key_double_db<"cwt", "wet", false>(settings);

  setup_input_output_gain();
}

Gate::~Gate() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Gate::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void Gate::process(std::span<float>& left_in,
                   std::span<float>& right_in,
                   std::span<float>& left_out,
                   std::span<float>& right_out,
                   std::span<float>& probe_left,
                   std::span<float>& probe_right) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out, probe_left, probe_right);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  /*
   This plugin gives the latency in number of samples
 */

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_port_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + util::to_string(latency_port_value, "") + " s");

    util::idle_add([=, this]() {
      if (!post_messages || latency.empty()) {
        return;
      }

      latency.emit();
    });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_port_value * 1000000000.0F);

    std::array<char, 1024U> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += buffer_duration;

    if (notification_dt >= notification_time_window) {
      attack_zone_start_port_value = lv2_wrapper->get_control_port_value("gzs");
      attack_threshold_port_value = lv2_wrapper->get_control_port_value("gt");
      release_zone_start_port_value = lv2_wrapper->get_control_port_value("hts");
      release_threshold_port_value = lv2_wrapper->get_control_port_value("hzs");
      reduction_port_value = lv2_wrapper->get_control_port_value("rlm");
      sidechain_port_value = lv2_wrapper->get_control_port_value("slm");
      curve_port_value = lv2_wrapper->get_control_port_value("clm");
      envelope_port_value = lv2_wrapper->get_control_port_value("elm");

      // Normalize the current gain reduction amount as a percentage,
      // where 0% is no gating, and 100% is a fully closed gate.
      // Double needed for the level bar widget.
      const double max_reduction_port_value = static_cast<double>(lv2_wrapper->get_control_port_value("gr"));
      // no reduction defaults to 1.0; aka db_to_linear(0 dB);
      gating_port_value = util::normalize(reduction_port_value, max_reduction_port_value);

      attack_zone_start.emit(attack_zone_start_port_value);
      attack_threshold.emit(attack_threshold_port_value);
      release_zone_start.emit(release_zone_start_port_value);
      release_threshold.emit(release_threshold_port_value);
      reduction.emit(reduction_port_value);
      sidechain.emit(sidechain_port_value);
      curve.emit(curve_port_value);
      envelope.emit(envelope_port_value);
      gating.emit(gating_port_value);

      notify();

      notification_dt = 0.0F;
    }
  }
}

void Gate::update_sidechain_links(const std::string& key) {
  if (util::gsettings_get_string(settings, "sidechain-input") != "External") {
    pm->destroy_links(list_proxies);

    list_proxies.clear();

    return;
  }

  const auto device_name = util::gsettings_get_string(settings, "sidechain-input-device");

  NodeInfo input_device = pm->ee_source_node;

  for (const auto& [serial, node] : pm->node_map) {
    if (node.name == device_name) {
      input_device = node;

      break;
    }
  }

  pm->destroy_links(list_proxies);

  list_proxies.clear();

  for (const auto& link : pm->link_nodes(input_device.id, get_node_id(), true)) {
    list_proxies.push_back(link);
  }
}

void Gate::update_probe_links() {
  update_sidechain_links("");
}

auto Gate::get_latency_seconds() -> float {
  return this->latency_port_value;
}
