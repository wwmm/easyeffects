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

#include "multiband_gate.hpp"
#include <qlist.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <QString>
#include <algorithm>
#include <format>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_multiband_gate.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "multi_band_macros.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_multiband_gate.hpp"
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
          tags::plugin_name::BaseName::multibandGate + "#" + instance_id)),
      frequency_range_end(n_bands),
      envelope_left(n_bands),
      envelope_right(n_bands),
      curve_left(n_bands),
      curve_right(n_bands),
      reduction_left(n_bands),
      reduction_right(n_bands) {
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/sc_mb_gate_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::MultibandGate>(settings);

  // specific plugin controls

  connect(settings, &db::MultibandGate::sidechainInputDeviceChanged, [&]() { update_sidechain_links(); });

  connect(settings, &db::MultibandGate::band0SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band1SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band2SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band3SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band4SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band5SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band6SidechainTypeChanged, [&]() { update_sidechain_links(); });
  connect(settings, &db::MultibandGate::band7SidechainTypeChanged, [&]() { update_sidechain_links(); });

  BIND_LV2_PORT("mode", gateMode, setGateMode, db::MultibandGate::gateModeChanged);
  BIND_LV2_PORT("envb", envelopeBoost, setEnvelopeBoost, db::MultibandGate::envelopeBoostChanged);
  BIND_LV2_PORT("ssplit", stereoSplit, setStereoSplit, db::MultibandGate::stereoSplitChanged);

  // dB controls with -inf mode.
  BIND_LV2_PORT_DB("g_dry", dry, setDry, db::MultibandGate::dryChanged, true);
  BIND_LV2_PORT_DB("g_wet", wet, setWet, db::MultibandGate::wetChanged, true);
  BIND_LV2_PORT_DB("in2lk", inputToLink, setInputToLink, db::MultibandGate::inputToLinkChanged, true);
  BIND_LV2_PORT_DB("in2sc", inputToSidechain, setInputToSidechain, db::MultibandGate::inputToSidechainChanged, true);
  BIND_LV2_PORT_DB("sc2in", sidechainToInput, setSidechainToInput, db::MultibandGate::sidechainToInputChanged, true);
  BIND_LV2_PORT_DB("sc2lk", sidechainToLink, setSidechainToLink, db::MultibandGate::sidechainToLinkChanged, true);
  BIND_LV2_PORT_DB("lk2sc", linkToSidechain, setLinkToSidechain, db::MultibandGate::linkToSidechainChanged, true);
  BIND_LV2_PORT_DB("lk2in", linkToInput, setLinkToInput, db::MultibandGate::linkToInputChanged, true);

  bind_bands();
}

MultibandGate::~MultibandGate() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void MultibandGate::reset() {
  settings->setDefaults();
}

// NOLINTNEXTLINE(readability-function-size,hicpp-function-size)
void MultibandGate::bind_bands() {
  using namespace tags::multiband_gate;

  BIND_LV2_PORT(cbe[1].data(), band1Enable, setBand1Enable, db::MultibandGate::band1EnableChanged);
  BIND_LV2_PORT(cbe[2].data(), band2Enable, setBand2Enable, db::MultibandGate::band2EnableChanged);
  BIND_LV2_PORT(cbe[3].data(), band3Enable, setBand3Enable, db::MultibandGate::band3EnableChanged);
  BIND_LV2_PORT(cbe[4].data(), band4Enable, setBand4Enable, db::MultibandGate::band4EnableChanged);
  BIND_LV2_PORT(cbe[5].data(), band5Enable, setBand5Enable, db::MultibandGate::band5EnableChanged);
  BIND_LV2_PORT(cbe[6].data(), band6Enable, setBand6Enable, db::MultibandGate::band6EnableChanged);
  BIND_LV2_PORT(cbe[7].data(), band7Enable, setBand7Enable, db::MultibandGate::band7EnableChanged);

  BIND_LV2_PORT(sf[1].data(), band1SplitFrequency, setBand1SplitFrequency,
                db::MultibandGate::band1SplitFrequencyChanged);
  BIND_LV2_PORT(sf[2].data(), band2SplitFrequency, setBand2SplitFrequency,
                db::MultibandGate::band2SplitFrequencyChanged);
  BIND_LV2_PORT(sf[3].data(), band3SplitFrequency, setBand3SplitFrequency,
                db::MultibandGate::band3SplitFrequencyChanged);
  BIND_LV2_PORT(sf[4].data(), band4SplitFrequency, setBand4SplitFrequency,
                db::MultibandGate::band4SplitFrequencyChanged);
  BIND_LV2_PORT(sf[5].data(), band5SplitFrequency, setBand5SplitFrequency,
                db::MultibandGate::band5SplitFrequencyChanged);
  BIND_LV2_PORT(sf[6].data(), band6SplitFrequency, setBand6SplitFrequency,
                db::MultibandGate::band6SplitFrequencyChanged);
  BIND_LV2_PORT(sf[7].data(), band7SplitFrequency, setBand7SplitFrequency,
                db::MultibandGate::band7SplitFrequencyChanged);

  BIND_BANDS_PROPERTY(ce, GateEnable, db::MultibandGate);
  BIND_BANDS_PROPERTY(bs, Solo, db::MultibandGate);
  BIND_BANDS_PROPERTY(bm, Mute, db::MultibandGate);
  BIND_BANDS_PROPERTY(gh, Hysteresis, db::MultibandGate);
  BIND_BANDS_PROPERTY(sce, SidechainType, db::MultibandGate);
  BIND_BANDS_PROPERTY(sclc, SidechainCustomLowcutFilter, db::MultibandGate);
  BIND_BANDS_PROPERTY(schc, SidechainCustomHighcutFilter, db::MultibandGate);
  BIND_BANDS_PROPERTY(sscs, StereoSplitSource, db::MultibandGate);
  BIND_BANDS_PROPERTY(scs, SidechainSource, db::MultibandGate);
  BIND_BANDS_PROPERTY(scm, SidechainMode, db::MultibandGate);
  BIND_BANDS_PROPERTY(at, AttackTime, db::MultibandGate);
  BIND_BANDS_PROPERTY(rt, ReleaseTime, db::MultibandGate);
  BIND_BANDS_PROPERTY(sclf, SidechainLowcutFrequency, db::MultibandGate);
  BIND_BANDS_PROPERTY(schf, SidechainHighcutFrequency, db::MultibandGate);
  BIND_BANDS_PROPERTY(sla, SidechainLookahead, db::MultibandGate);
  BIND_BANDS_PROPERTY(scr, SidechainReactivity, db::MultibandGate);

  BIND_BANDS_PROPERTY_DB(ht, HysteresisThreshold, db::MultibandGate, false);
  BIND_BANDS_PROPERTY_DB(hz, HysteresisZone, db::MultibandGate, false);
  BIND_BANDS_PROPERTY_DB(gt, CurveThreshold, db::MultibandGate, false);
  BIND_BANDS_PROPERTY_DB(gz, CurveZone, db::MultibandGate, false);
  BIND_BANDS_PROPERTY_DB(gr, Reduction, db::MultibandGate, false);
  BIND_BANDS_PROPERTY_DB(mk, Makeup, db::MultibandGate, false);

  // dB controls with -inf mode.
  BIND_BANDS_PROPERTY_DB(scp, SidechainPreamp, db::MultibandGate, true);
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

    for (uint n = 0U; n < n_bands; n++) {
      const auto nstr = util::to_string(n);

      frequency_range_end[n] = lv2_wrapper->get_control_port_value("fre_" + nstr);

      envelope_left[n] = util::linear_to_db(lv2_wrapper->get_control_port_value("elm_" + nstr + "l"));
      envelope_right[n] = util::linear_to_db(lv2_wrapper->get_control_port_value("elm_" + nstr + "r"));

      curve_left[n] = util::linear_to_db(lv2_wrapper->get_control_port_value("clm_" + nstr + "l"));
      curve_right[n] = util::linear_to_db(lv2_wrapper->get_control_port_value("clm_" + nstr + "r"));

      reduction_left[n] = util::linear_to_db(lv2_wrapper->get_control_port_value("rlm_" + nstr + "l"));
      reduction_right[n] = util::linear_to_db(lv2_wrapper->get_control_port_value("rlm_" + nstr + "r"));
    }
  }
}

void MultibandGate::update_sidechain_links() {
  using namespace tags::multiband_gate;

  auto external_sidechain_enabled = false;

  for (uint n = 0U; !external_sidechain_enabled && n < n_bands; n++) {
    external_sidechain_enabled =
        settings->defaultSidechainTypeLabelsValue()[settings->property(band_sidechain_type[n].data()).value<int>()] ==
        "External";
  }

  settings->setExternalSidechainEnabled(external_sidechain_enabled);

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

QList<float> MultibandGate::getFrequencyRangeEnd() const {
  return frequency_range_end;
}

QList<float> MultibandGate::getEnvelopeLevelLeft() const {
  return envelope_left;
}

QList<float> MultibandGate::getEnvelopeLevelRight() const {
  return envelope_right;
}

QList<float> MultibandGate::getCurveLevelLeft() const {
  return curve_left;
}

QList<float> MultibandGate::getCurveLevelRight() const {
  return curve_right;
}

QList<float> MultibandGate::getReductionLevelLeft() const {
  return reduction_left;
}

QList<float> MultibandGate::getReductionLevelRight() const {
  return reduction_right;
}
