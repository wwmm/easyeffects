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

#include "compressor.hpp"

Compressor::Compressor(const std::string& tag,
                       const std::string& schema,
                       const std::string& schema_path,
                       PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::compressor, schema, schema_path, pipe_manager, true),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/sc_compressor_stereo")) {
  if (!lv2_wrapper->found_plugin) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/sc_compressor_stereo is not installed");
  }

  gconnections.push_back(g_signal_connect(settings, "changed::sidechain-type",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Compressor*>(user_data);

                                            self->update_sidechain_links(key);
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(settings, "changed::sidechain-input-device",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Compressor*>(user_data);

                                            self->update_sidechain_links(key);
                                          }),
                                          this));

  lv2_wrapper->bind_key_enum<"cm", "mode">(settings);

  lv2_wrapper->bind_key_enum<"sct", "sidechain-type">(settings);

  lv2_wrapper->bind_key_enum<"scm", "sidechain-mode">(settings);

  lv2_wrapper->bind_key_enum<"scs", "sidechain-source">(settings);

  lv2_wrapper->bind_key_enum<"shpm", "hpf-mode">(settings);

  lv2_wrapper->bind_key_enum<"slpm", "lpf-mode">(settings);

  lv2_wrapper->bind_key_bool<"scl", "sidechain-listen">(settings);

  lv2_wrapper->bind_key_double<"at", "attack">(settings);

  lv2_wrapper->bind_key_double<"rt", "release">(settings);

  lv2_wrapper->bind_key_double<"cr", "ratio">(settings);

  lv2_wrapper->bind_key_double<"scr", "sidechain-reactivity">(settings);

  lv2_wrapper->bind_key_double<"sla", "sidechain-lookahead">(settings);

  lv2_wrapper->bind_key_double<"shpf", "hpf-frequency">(settings);

  lv2_wrapper->bind_key_double<"slpf", "lpf-frequency">(settings);

  lv2_wrapper->bind_key_double_db<"rrl", "release-threshold">(settings);

  lv2_wrapper->bind_key_double_db<"bth", "boost-threshold">(settings);

  lv2_wrapper->bind_key_double_db<"bsa", "boost-amount">(settings);

  lv2_wrapper->bind_key_double_db<"al", "threshold">(settings);

  lv2_wrapper->bind_key_double_db<"kn", "knee">(settings);

  lv2_wrapper->bind_key_double_db<"mk", "makeup">(settings);

  lv2_wrapper->bind_key_double_db<"scp", "sidechain-preamp">(settings);

  setup_input_output_gain();
}

Compressor::~Compressor() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Compressor::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void Compressor::process(std::span<float>& left_in,
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

    util::debug(log_tag + name + " latency: " + std::to_string(latency_port_value) + " s");

    g_idle_add((GSourceFunc) +
                   [](gpointer user_data) {
                     if (!post_messages) {
                       return G_SOURCE_REMOVE;
                     }

                     auto* self = static_cast<Compressor*>(user_data);

                     if (self->latency.empty()) {
                       return G_SOURCE_REMOVE;
                     }

                     self->latency.emit(self->latency_port_value);

                     return G_SOURCE_REMOVE;
                   },
               this);

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
      reduction_port_value = lv2_wrapper->get_control_port_value("rlm");
      sidechain_port_value = lv2_wrapper->get_control_port_value("slm");
      curve_port_value = lv2_wrapper->get_control_port_value("clm");
      envelope_port_value = lv2_wrapper->get_control_port_value("elm");

      g_idle_add((GSourceFunc) +
                     [](gpointer user_data) {
                       if (!post_messages) {
                         return G_SOURCE_REMOVE;
                       }

                       auto* self = static_cast<Compressor*>(user_data);

                       if (self->reduction.empty() || self->sidechain.empty() || self->curve.empty() ||
                           self->envelope.empty()) {
                         return G_SOURCE_REMOVE;
                       }

                       self->reduction.emit(self->reduction_port_value);
                       self->sidechain.emit(self->sidechain_port_value);
                       self->curve.emit(self->curve_port_value);
                       self->envelope.emit(self->envelope_port_value);

                       return G_SOURCE_REMOVE;
                     },
                 this);

      notify();

      notification_dt = 0.0F;
    }
  }
}

void Compressor::update_sidechain_links(const Glib::ustring& key) {
  if (g_strcmp0(g_settings_get_string(settings, "sidechain-type"), "External") == 0) {
    const auto device_name = std::string(g_settings_get_string(settings, "sidechain-input-device"));

    NodeInfo input_device = pm->ee_source_node;

    for (const auto& [ts, node] : pm->node_map) {
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
  } else {
    pm->destroy_links(list_proxies);

    list_proxies.clear();
  }
}

void Compressor::update_probe_links() {
  update_sidechain_links("");
}