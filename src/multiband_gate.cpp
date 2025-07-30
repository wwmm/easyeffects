/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include "multiband_gate.hpp"
#include <qtypes.h>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_multiband_gate.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "spa/utils/defs.h"
#include "tags_plugin_name.hpp"
#include "util.hpp"

MultibandGate::MultibandGate(const std::string& tag,
                             pw::Manager* pipe_manager,
                             PipelineType pipe_type,
                             QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::multibandGate,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type,
                 true),
      settings(db::Manager::self().get_plugin_db<db::MultibandGate>(
          pipe_type,
          tags::plugin_name::BaseName::multibandGate + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/sc_mb_gate_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  package_installed = lv2_wrapper->found_plugin;

  if (!package_installed) {
    util::debug(log_tag + lv2_plugin_uri + " is not installed");
  }

  init_common_controls<db::MultibandGate>(settings);

  // specific plugin controls

  connect(settings, &db::MultibandGate::sidechainInputDeviceChanged, [&]() { update_sidechain_links(); });

  connect(settings, &db::MultibandGate::sidechainType0Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType1Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType2Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType3Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType4Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType5Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType6Changed, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::sidechainType7Changed, [&]() { update_sidechain_links(); });

  BIND_LV2_PORT("mode", gateMode, setGateMode, db::MultibandGate::gateModeChanged);
  BIND_LV2_PORT("envb", envelopeBoost, setEnvelopeBoost, db::MultibandGate::envelopeBoostChanged);
  BIND_LV2_PORT("ssplit", stereoSplit, setStereoSplit, db::MultibandGate::stereoSplitChanged);

  // dB controls with -inf mode.
  BIND_LV2_PORT_DB("cdr", dry, setDry, db::MultibandGate::dryChanged, true);
  BIND_LV2_PORT_DB("cwt", wet, setWet, db::MultibandGate::wetChanged, true);

  // TODO: set band parameters
  // ...
}

MultibandGate::~MultibandGate() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(log_tag + name.toStdString() + " destroyed");
}

void MultibandGate::reset() {
  settings->setDefaults();
}

void MultibandGate::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void MultibandGate::process([[maybe_unused]] std::span<float>& left_in,
                            [[maybe_unused]] std::span<float>& right_in,
                            [[maybe_unused]] std::span<float>& left_out,
                            [[maybe_unused]] std::span<float>& right_out) {}

void MultibandGate::process(std::span<float>& left_in,
                            std::span<float>& right_in,
                            std::span<float>& left_out,
                            std::span<float>& right_out,
                            std::span<float>& probe_left,
                            std::span<float>& probe_right) {}

void MultibandGate::update_sidechain_links() {
  auto external_sidechain_enabled = false;

  for (uint n = 0U; !external_sidechain_enabled && n < n_bands; n++) {
    const auto band_name = "sidechainType" + util::to_string(n);

    external_sidechain_enabled =
        settings->defaultSidechainTypeLabelsValue()[settings->property(band_name.c_str()).value<int>()] == "External";
  }

  if (!external_sidechain_enabled) {
    pm->destroy_links(list_proxies);

    list_proxies.clear();

    return;
  }

  const auto device_name = settings->sidechainInputDevice();

  auto input_device = pm->model_nodes.get_node_by_name(device_name);

  input_device = input_device.serial == SPA_ID_INVALID ? pm->ee_source_node : input_device;

  pm->destroy_links(list_proxies);

  list_proxies.clear();

  for (const auto& link : pm->link_nodes(input_device.id, get_node_id(), true)) {
    list_proxies.push_back(link);
  }
}

void MultibandGate::update_probe_links() {
  update_sidechain_links();
}

auto MultibandGate::get_latency_seconds() -> float {
  return this->latency_value;
}
