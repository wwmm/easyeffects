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

#include "limiter.hpp"
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <QApplication>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
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
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/sc_limiter_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Limiter>(settings);

  // specific plugin controls

  connect(settings, &db::Limiter::sidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::Limiter::sidechainInputDeviceChanged, [&]() { update_sidechain_links(); });

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
  BIND_LV2_PORT("smooth", alrKneeSmooth, setAlrKneeSmooth, db::Limiter::alrKneeSmoothChanged);
  BIND_LV2_PORT("extsc", sidechainType, setSidechainType, db::Limiter::sidechainTypeChanged);
  BIND_LV2_PORT_DB("th", threshold, setThreshold, db::Limiter::thresholdChanged, false);
  BIND_LV2_PORT_DB("knee", alrKnee, setAlrKnee, db::Limiter::alrKneeChanged, false);

  // dB controls with -inf mode.
  BIND_LV2_PORT_DB("scp", sidechainPreamp, setSidechainPreamp, db::Limiter::sidechainPreampChanged, true);
  BIND_LV2_PORT_DB("in2lk", inputToLink, setInputToLink, db::Limiter::inputToLinkChanged, true);
  BIND_LV2_PORT_DB("in2sc", inputToSidechain, setInputToSidechain, db::Limiter::inputToSidechainChanged, true);
  BIND_LV2_PORT_DB("sc2in", sidechainToInput, setSidechainToInput, db::Limiter::sidechainToInputChanged, true);
  BIND_LV2_PORT_DB("sc2lk", sidechainToLink, setSidechainToLink, db::Limiter::sidechainToLinkChanged, true);
  BIND_LV2_PORT_DB("lk2sc", linkToSidechain, setLinkToSidechain, db::Limiter::linkToSidechainChanged, true);
  BIND_LV2_PORT_DB("lk2in", linkToInput, setLinkToInput, db::Limiter::linkToInputChanged, true);
}

Limiter::~Limiter() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Limiter::reset() {
  settings->setDefaults();
}

void Limiter::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  ready = false;

  lv2_wrapper->set_n_samples(n_samples);

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      QApplication::instance(),
      [this] {
        lv2_wrapper->create_instance(rate);

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
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

    gain_l_port_value = util::linear_to_db(lv2_wrapper->get_control_port_value("grlm_l"));
    gain_r_port_value = util::linear_to_db(lv2_wrapper->get_control_port_value("grlm_r"));
    sidechain_l_port_value = util::linear_to_db(lv2_wrapper->get_control_port_value("sclm_l"));
    sidechain_r_port_value = util::linear_to_db(lv2_wrapper->get_control_port_value("sclm_r"));
  }
}

void Limiter::update_sidechain_links() {
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
