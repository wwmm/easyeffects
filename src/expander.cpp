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

#include "expander.hpp"
#include <qnamespace.h>
#include <qobject.h>
#include <spa/utils/defs.h>
#include <sys/types.h>
#include <QApplication>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_expander.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Expander::Expander(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::expander,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type,
                 true),
      settings(
          db::Manager::self().get_plugin_db<db::Expander>(pipe_type,
                                                          tags::plugin_name::BaseName::expander + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/sc_expander_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Expander>(settings);

  connect(settings, &db::Expander::sidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::Expander::sidechainInputDeviceChanged, [&]() { update_sidechain_links(); });

  BIND_LV2_PORT("em", mode, setMode, db::Expander::modeChanged);
  BIND_LV2_PORT("sci", sidechainType, setSidechainType, db::Expander::sidechainTypeChanged);
  BIND_LV2_PORT("scm", sidechainMode, setSidechainMode, db::Expander::sidechainModeChanged);
  BIND_LV2_PORT("scs", sidechainSource, setSidechainSource, db::Expander::sidechainSourceChanged);
  BIND_LV2_PORT("sscs", stereoSplitSource, setStereoSplitSource, db::Expander::stereoSplitSourceChanged);
  BIND_LV2_PORT("shpm", hpfMode, setHpfMode, db::Expander::hpfModeChanged);
  BIND_LV2_PORT("slpm", lpfMode, setLpfMode, db::Expander::lpfModeChanged);
  BIND_LV2_PORT("ssplit", stereoSplit, setStereoSplit, db::Expander::stereoSplitChanged);
  BIND_LV2_PORT("scl", sidechainListen, setSidechainListen, db::Expander::sidechainListenChanged);
  BIND_LV2_PORT("at", attack, setAttack, db::Expander::attackChanged);
  BIND_LV2_PORT("rt", release, setRelease, db::Expander::releaseChanged);
  BIND_LV2_PORT("er", ratio, setRatio, db::Expander::ratioChanged);
  BIND_LV2_PORT("scr", sidechainReactivity, setSidechainReactivity, db::Expander::sidechainReactivityChanged);
  BIND_LV2_PORT("sla", sidechainLookahead, setSidechainLookahead, db::Expander::sidechainLookaheadChanged);
  BIND_LV2_PORT_DB("al", threshold, setThreshold, db::Expander::thresholdChanged, false);
  BIND_LV2_PORT_DB("kn", knee, setKnee, db::Expander::kneeChanged, false);
  BIND_LV2_PORT_DB("mk", makeup, setMakeup, db::Expander::makeupChanged, false);
  BIND_LV2_PORT("shpf", hpfFrequency, setHpfFrequency, db::Expander::hpfFrequencyChanged);
  BIND_LV2_PORT("slpf", lpfFrequency, setLpfFrequency, db::Expander::lpfFrequencyChanged);

  // dB controls with -inf mode.
  BIND_LV2_PORT_DB("rrl", releaseThreshold, setReleaseThreshold, db::Expander::releaseThresholdChanged, true);
  BIND_LV2_PORT_DB("cdr", dry, setDry, db::Expander::dryChanged, true);
  BIND_LV2_PORT_DB("cwt", wet, setWet, db::Expander::wetChanged, true);
  BIND_LV2_PORT_DB("scp", sidechainPreamp, setSidechainPreamp, db::Expander::sidechainPreampChanged, true);
  BIND_LV2_PORT_DB("in2lk", inputToLink, setInputToLink, db::Expander::inputToLinkChanged, true);
  BIND_LV2_PORT_DB("in2sc", inputToSidechain, setInputToSidechain, db::Expander::inputToSidechainChanged, true);
  BIND_LV2_PORT_DB("sc2in", sidechainToInput, setSidechainToInput, db::Expander::sidechainToInputChanged, true);
  BIND_LV2_PORT_DB("sc2lk", sidechainToLink, setSidechainToLink, db::Expander::sidechainToLinkChanged, true);
  BIND_LV2_PORT_DB("lk2sc", linkToSidechain, setLinkToSidechain, db::Expander::linkToSidechainChanged, true);
  BIND_LV2_PORT_DB("lk2in", linkToInput, setLinkToInput, db::Expander::linkToInputChanged, true);
}

Expander::~Expander() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Expander::reset() {
  settings->setDefaults();
}

void Expander::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void Expander::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  ready = false;

  lv2_wrapper->set_n_samples(n_samples);

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        lv2_wrapper->create_instance(rate);

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Expander::process([[maybe_unused]] std::span<float>& left_in,
                       [[maybe_unused]] std::span<float>& right_in,
                       [[maybe_unused]] std::span<float>& left_out,
                       [[maybe_unused]] std::span<float>& right_out) {}

void Expander::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out,
                       std::span<float>& probe_left,
                       std::span<float>& probe_right) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (!ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    if (output_gain != 1.0F) {
      apply_gain(left_out, right_out, output_gain);
    }

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
  }
}

void Expander::update_sidechain_links() {
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

void Expander::update_probe_links() {
  update_sidechain_links();
}

auto Expander::get_latency_seconds() -> float {
  return this->latency_value;
}

float Expander::getReductionLevelLeft() const {
  return reduction_left;
}

float Expander::getReductionLevelRight() const {
  return reduction_right;
}

float Expander::getSideChainLevelLeft() const {
  return sidechain_left;
}

float Expander::getSideChainLevelRight() const {
  return sidechain_right;
}

float Expander::getCurveLevelLeft() const {
  return curve_left;
}

float Expander::getCurveLevelRight() const {
  return curve_right;
}

float Expander::getEnvelopeLevelLeft() const {
  return envelope_left;
}

float Expander::getEnvelopeLevelRight() const {
  return envelope_right;
}
