/*
 *  Copyright © 2017-2024 Wellington Wallace
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

#include "limiter.hpp"
#include <sys/types.h>
#include <algorithm>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_limiter.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Limiter::Limiter(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::limiter,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type,
                 true),
      settings(
          db::Manager::self().get_plugin_db<db::Limiter>(pipe_type,
                                                         tags::plugin_name::BaseName::limiter + "#" + instance_id)) {
  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/sc_limiter_stereo");

  package_installed = lv2_wrapper->found_plugin;

  if (!package_installed) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/sc_limiter_stereo is not installed");
  }

  init_common_controls<db::Limiter>(settings);

  // specific plugin controls

  //   gconnections.push_back(g_signal_connect(settings, "changed::external-sidechain",
  //                                           G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                                             auto* self = static_cast<Limiter*>(user_data);

  //                                             self->update_sidechain_links(key);
  //                                           }),
  //                                           this));

  //   gconnections.push_back(g_signal_connect(settings, "changed::sidechain-input-device",
  //                                           G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                                             auto* self = static_cast<Limiter*>(user_data);

  //                                             self->update_sidechain_links(key);
  //                                           }),
  //                                           this));

  //   lv2_wrapper->bind_key_double_db<"th", "threshold">(settings);

  //   lv2_wrapper->bind_key_double_db<"scp", "sidechain-preamp">(settings);

  //   lv2_wrapper->bind_key_double_db<"knee", "alr-knee">(settings);

  BIND_LV2_PORT("mode", mode, setMode, db::Limiter::modeChanged);
  BIND_LV2_PORT("ovs", oversampling, setOversampling, db::Limiter::oversamplingChanged);
  BIND_LV2_PORT("dith", dithering, setDithering, db::Limiter::ditheringChanged);
  BIND_LV2_PORT("lk", lookahead, setLookahead, db::Limiter::lookaheadChanged);
  BIND_LV2_PORT("at", attack, setAttack, db::Limiter::attackChanged);
  BIND_LV2_PORT("rt", release, setRelease, db::Limiter::releaseChanged);
  BIND_LV2_PORT("boost", gainBoost, setGainBoost, db::Limiter::gainBoostChanged);
  BIND_LV2_PORT("slink", stereoLink, setStereoLink, db::Limiter::stereoLinkChanged);
  BIND_LV2_PORT("alr", alr, setAlr, db::Limiter::alrChanged);
  BIND_LV2_PORT("alr_at", alrAttack, setAlrAttack, db::Limiter::alrAttackChanged);
  BIND_LV2_PORT("alr_rt", alrRelease, setAlrRelease, db::Limiter::alrReleaseChanged);
  BIND_LV2_PORT("extsc", externalSidechain, setExternalSidechain, db::Limiter::externalSidechainChanged);
}

Limiter::~Limiter() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name.toStdString() + " destroyed");
}

void Limiter::reset() {
  settings->setDefaults();
}

void Limiter::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void Limiter::process([[maybe_unused]] std::span<float>& left_in,
                      [[maybe_unused]] std::span<float>& right_in,
                      [[maybe_unused]] std::span<float>& left_out,
                      [[maybe_unused]] std::span<float>& right_out) {}

void Limiter::process(std::span<float>& left_in,
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

    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name.toStdString() + " latency: " + util::to_string(latency_value, "") + " s");

    update_filter_params();
  }

  get_peaks(left_in, right_in, left_out, right_out);

  gain_l_port_value = lv2_wrapper->get_control_port_value("grlm_l");
  gain_r_port_value = lv2_wrapper->get_control_port_value("grlm_r");
  sidechain_l_port_value = lv2_wrapper->get_control_port_value("sclm_l");
  sidechain_r_port_value = lv2_wrapper->get_control_port_value("sclm_r");
}

void Limiter::update_sidechain_links() {
  if (!settings->externalSidechain()) {
    pm->destroy_links(list_proxies);

    list_proxies.clear();

    return;
  }

  const auto device_name = settings->sidechainInputDevice();

  auto input_device = pm->ee_source_node;

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

void Limiter::update_probe_links() {
  update_sidechain_links();
}

auto Limiter::get_latency_seconds() -> float {
  return this->latency_value;
}

float Limiter::getGainLevelLeft() const {
  return this->gain_l_port_value;
}

float Limiter::getGainLevelRight() const {
  return this->gain_r_port_value;
}

float Limiter::getSideChainLevelLeft() const {
  return this->sidechain_l_port_value;
}

float Limiter::getSideChainLevelRight() const {
  return this->sidechain_r_port_value;
}