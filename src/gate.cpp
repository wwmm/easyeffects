/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "gate.hpp"
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <algorithm>
#include <format>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_gate.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "pw_objects.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Gate::Gate(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::gate,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type,
                 true),
      settings(db::Manager::self().get_plugin_db<db::Gate>(pipe_type,
                                                           tags::plugin_name::BaseName::gate + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/sc_gate_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Gate>(settings);

  // specific plugin controls

  connect(settings, &db::Gate::sidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::Gate::sidechainInputDeviceChanged, [&]() { update_sidechain_links(); });

  BIND_LV2_PORT("sci", sidechainType, setSidechainType, db::Gate::sidechainTypeChanged);
  BIND_LV2_PORT("scm", sidechainMode, setSidechainMode, db::Gate::sidechainModeChanged);
  BIND_LV2_PORT("scl", sidechainListen, setSidechainListen, db::Gate::sidechainListenChanged);
  BIND_LV2_PORT("scs", sidechainSource, setSidechainSource, db::Gate::sidechainSourceChanged);
  BIND_LV2_PORT("sscs", stereoSplitSource, setStereoSplitSource, db::Gate::stereoSplitSourceChanged);
  BIND_LV2_PORT("ssplit", stereoSplit, setStereoSplit, db::Gate::stereoSplitChanged);
  BIND_LV2_PORT("scr", sidechainReactivity, setSidechainReactivity, db::Gate::sidechainReactivityChanged);
  BIND_LV2_PORT("sla", sidechainLookahead, setSidechainLookahead, db::Gate::sidechainLookaheadChanged);
  BIND_LV2_PORT("shpm", hpfMode, setHpfMode, db::Gate::hpfModeChanged);
  BIND_LV2_PORT("slpm", lpfMode, setLpfMode, db::Gate::lpfModeChanged);
  BIND_LV2_PORT("shpf", hpfFrequency, setHpfFrequency, db::Gate::hpfFrequencyChanged);
  BIND_LV2_PORT("slpf", lpfFrequency, setLpfFrequency, db::Gate::lpfFrequencyChanged);
  BIND_LV2_PORT("at", attack, setAttack, db::Gate::attackChanged);
  BIND_LV2_PORT("rt", release, setRelease, db::Gate::releaseChanged);
  BIND_LV2_PORT("gh", hysteresis, setHysteresis, db::Gate::hysteresisChanged);
  BIND_LV2_PORT_DB("mk", makeup, setMakeup, db::Gate::makeupChanged, false);
  BIND_LV2_PORT_DB("gr", reduction, setReduction, db::Gate::reductionChanged, false);
  BIND_LV2_PORT_DB("gt", curveThreshold, setCurveThreshold, db::Gate::curveThresholdChanged, false);
  BIND_LV2_PORT_DB("gz", curveZone, setCurveZone, db::Gate::curveZoneChanged, false);
  BIND_LV2_PORT_DB("ht", hysteresisThreshold, setHysteresisThreshold, db::Gate::hysteresisThresholdChanged, false);
  BIND_LV2_PORT_DB("hz", hysteresisZone, setHysteresisZone, db::Gate::hysteresisZoneChanged, false);

  // dB controls with -inf mode.
  BIND_LV2_PORT_DB("cdr", dry, setDry, db::Gate::dryChanged, true);
  BIND_LV2_PORT_DB("cwt", wet, setWet, db::Gate::wetChanged, true);
  BIND_LV2_PORT_DB("scp", sidechainPreamp, setSidechainPreamp, db::Gate::sidechainPreampChanged, true);
  BIND_LV2_PORT_DB("in2lk", inputToLink, setInputToLink, db::Gate::inputToLinkChanged, true);
  BIND_LV2_PORT_DB("in2sc", inputToSidechain, setInputToSidechain, db::Gate::inputToSidechainChanged, true);
  BIND_LV2_PORT_DB("sc2in", sidechainToInput, setSidechainToInput, db::Gate::sidechainToInputChanged, true);
  BIND_LV2_PORT_DB("sc2lk", sidechainToLink, setSidechainToLink, db::Gate::sidechainToLinkChanged, true);
  BIND_LV2_PORT_DB("lk2sc", linkToSidechain, setLinkToSidechain, db::Gate::linkToSidechainChanged, true);
  BIND_LV2_PORT_DB("lk2in", linkToInput, setLinkToInput, db::Gate::linkToInputChanged, true);
}

Gate::~Gate() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Gate::reset() {
  settings->setDefaults();
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

void Gate::process([[maybe_unused]] std::span<float>& left_in,
                   [[maybe_unused]] std::span<float>& right_in,
                   [[maybe_unused]] std::span<float>& left_out,
                   [[maybe_unused]] std::span<float>& right_out) {}

void Gate::process(std::span<float>& left_in,
                   std::span<float>& right_in,
                   std::span<float>& left_out,
                   std::span<float>& right_out,
                   std::span<float>& probe_left,
                   std::span<float>& probe_right) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

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

  // This plugin gives the latency in number of samples

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);

    reduction_left = util::linear_to_db(lv2_wrapper->get_control_port_value("rlm_l"));
    reduction_right = util::linear_to_db(lv2_wrapper->get_control_port_value("rlm_r"));

    sidechain_left = util::linear_to_db(lv2_wrapper->get_control_port_value("slm_l"));
    sidechain_right = util::linear_to_db(lv2_wrapper->get_control_port_value("slm_r"));

    curve_left = util::linear_to_db(lv2_wrapper->get_control_port_value("clm_l"));
    curve_right = util::linear_to_db(lv2_wrapper->get_control_port_value("clm_r"));

    envelope_left = util::linear_to_db(lv2_wrapper->get_control_port_value("elm_l"));
    envelope_right = util::linear_to_db(lv2_wrapper->get_control_port_value("elm_r"));

    attack_zone_start = util::linear_to_db(lv2_wrapper->get_control_port_value("gzs"));
    attack_threshold = util::linear_to_db(lv2_wrapper->get_control_port_value("gt"));

    release_zone_start = util::linear_to_db(lv2_wrapper->get_control_port_value("hts"));
    release_threshold = util::linear_to_db(lv2_wrapper->get_control_port_value("hzs"));
  }
}

void Gate::update_sidechain_links() {
  if (settings->defaultSidechainTypeLabelsValue()[settings->sidechainType()] != "External") {
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

void Gate::update_probe_links() {
  update_sidechain_links();
}

auto Gate::get_latency_seconds() -> float {
  return this->latency_value;
}

float Gate::getReductionLevelLeft() const {
  return reduction_left;
}

float Gate::getReductionLevelRight() const {
  return reduction_right;
}

float Gate::getSideChainLevelLeft() const {
  return sidechain_left;
}

float Gate::getSideChainLevelRight() const {
  return sidechain_right;
}

float Gate::getCurveLevelLeft() const {
  return curve_left;
}

float Gate::getCurveLevelRight() const {
  return curve_right;
}

float Gate::getEnvelopeLevelLeft() const {
  return envelope_left;
}

float Gate::getEnvelopeLevelRight() const {
  return envelope_right;
}

float Gate::getAttackZoneStart() const {
  return attack_zone_start;
}

float Gate::getAttackThreshold() const {
  return attack_threshold;
}

float Gate::getReleaseZoneStart() const {
  return release_zone_start;
}

float Gate::getReleaseThreshold() const {
  return release_threshold;
}
