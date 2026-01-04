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

#include "gate_preset.hpp"
#include <qstringview.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_gate.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

GatePreset::GatePreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Gate>(pipeline_type);
}

void GatePreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["dry"] = settings->dry();

  json[section][instance_name]["wet"] = settings->wet();

  json[section][instance_name]["attack"] = settings->attack();

  json[section][instance_name]["release"] = settings->release();

  json[section][instance_name]["curve-threshold"] = settings->curveThreshold();

  json[section][instance_name]["curve-zone"] = settings->curveZone();

  json[section][instance_name]["hysteresis"] = settings->hysteresis();

  json[section][instance_name]["hysteresis-threshold"] = settings->hysteresisThreshold();

  json[section][instance_name]["hysteresis-zone"] = settings->hysteresisZone();

  json[section][instance_name]["reduction"] = settings->reduction();

  json[section][instance_name]["makeup"] = settings->makeup();

  json[section][instance_name]["stereo-split"] = settings->stereoSplit();

  json[section][instance_name]["sidechain"]["type"] =
      settings->defaultSidechainTypeLabelsValue()[settings->sidechainType()].toStdString();

  json[section][instance_name]["sidechain"]["mode"] =
      settings->defaultSidechainModeLabelsValue()[settings->sidechainMode()].toStdString();

  json[section][instance_name]["sidechain"]["source"] =
      settings->defaultSidechainSourceLabelsValue()[settings->sidechainSource()].toStdString();

  json[section][instance_name]["sidechain"]["stereo-split-source"] =
      settings->defaultStereoSplitSourceLabelsValue()[settings->stereoSplitSource()].toStdString();

  json[section][instance_name]["sidechain"]["preamp"] = settings->sidechainPreamp();

  json[section][instance_name]["sidechain"]["reactivity"] = settings->sidechainReactivity();

  json[section][instance_name]["sidechain"]["lookahead"] = settings->sidechainLookahead();

  json[section][instance_name]["hpf-mode"] = settings->defaultHpfModeLabelsValue()[settings->hpfMode()].toStdString();

  json[section][instance_name]["hpf-frequency"] = settings->hpfFrequency();

  json[section][instance_name]["lpf-mode"] = settings->defaultLpfModeLabelsValue()[settings->lpfMode()].toStdString();

  json[section][instance_name]["lpf-frequency"] = settings->lpfFrequency();

  json[section][instance_name]["input-to-sidechain"] = settings->inputToSidechain();

  json[section][instance_name]["input-to-link"] = settings->inputToLink();

  json[section][instance_name]["sidechain-to-input"] = settings->sidechainToInput();

  json[section][instance_name]["sidechain-to-link"] = settings->sidechainToLink();

  json[section][instance_name]["link-to-input"] = settings->linkToInput();

  json[section][instance_name]["link-to-sidechain"] = settings->linkToSidechain();
}

void GatePreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("dry", Dry);
  UPDATE_PROPERTY("wet", Wet);
  UPDATE_PROPERTY("attack", Attack);
  UPDATE_PROPERTY("release", Release);
  UPDATE_PROPERTY("curve-threshold", CurveThreshold);
  UPDATE_PROPERTY("curve-zone", CurveZone);
  UPDATE_PROPERTY("hysteresis", Hysteresis);
  UPDATE_PROPERTY("hysteresis-threshold", HysteresisThreshold);
  UPDATE_PROPERTY("hysteresis-zone", HysteresisZone);
  UPDATE_PROPERTY("reduction", Reduction);
  UPDATE_PROPERTY("makeup", Makeup);
  UPDATE_PROPERTY("reduction", Reduction);
  UPDATE_PROPERTY("stereo-split", StereoSplit);
  UPDATE_PROPERTY("hpf-frequency", HpfFrequency);
  UPDATE_PROPERTY("lpf-frequency", LpfFrequency);
  UPDATE_PROPERTY("input-to-sidechain", InputToSidechain);
  UPDATE_PROPERTY("input-to-link", InputToLink);
  UPDATE_PROPERTY("sidechain-to-input", SidechainToInput);
  UPDATE_PROPERTY("sidechain-to-link", SidechainToLink);
  UPDATE_PROPERTY("link-to-input", LinkToInput);
  UPDATE_PROPERTY("link-to-sidechain", LinkToSidechain);

  UPDATE_ENUM_LIKE_PROPERTY("hpf-mode", HpfMode);
  UPDATE_ENUM_LIKE_PROPERTY("lpf-mode", LpfMode);

  UPDATE_PROPERTY_INSIDE_SUBSECTION("sidechain", "preamp", SidechainPreamp);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("sidechain", "reactivity", SidechainReactivity);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("sidechain", "lookahead", SidechainLookahead);

  UPDATE_ENUM_LIKE_PROPERTY_INSIDE_SUBSECTION("sidechain", "type", SidechainType);
  UPDATE_ENUM_LIKE_PROPERTY_INSIDE_SUBSECTION("sidechain", "mode", SidechainMode);
  UPDATE_ENUM_LIKE_PROPERTY_INSIDE_SUBSECTION("sidechain", "source", SidechainSource);
  UPDATE_ENUM_LIKE_PROPERTY_INSIDE_SUBSECTION("sidechain", "stereo-split-source", StereoSplitSource);
}
