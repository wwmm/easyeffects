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

#include "echo_canceller_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_echo_canceller.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

EchoCancellerPreset::EchoCancellerPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::EchoCanceller>(pipeline_type);
}

void EchoCancellerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["echo-canceller"]["enable"] = settings->enableEchoCanceller();
  json[section][instance_name]["echo-canceller"]["mobile-mode"] = settings->echoCancellerMobileMode();
  json[section][instance_name]["echo-canceller"]["enforce-high-pass"] = settings->echoCancellerEnforceHighPass();
  json[section][instance_name]["echo-canceller"]["automatic-gain-control"] = settings->enableAGC();

  json[section][instance_name]["noise-suppression"]["enable"] = settings->enableNoiseSuppression();
  json[section][instance_name]["noise-suppression"]["level"] =
      settings->defaultNoiseSuppressionLevelLabelsValue()[settings->noiseSuppressionLevel()].toStdString();

  json[section][instance_name]["high-pass"]["enable"] = settings->enableHighPassFilter();
  json[section][instance_name]["high-pass"]["full-band"] = settings->highPassFilterFullBand();
}

void EchoCancellerPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);

  UPDATE_PROPERTY_INSIDE_SUBSECTION("echo-canceller", "enable", EnableEchoCanceller);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("echo-canceller", "mobile-mode", EchoCancellerMobileMode);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("echo-canceller", "enforce-high-pass", EchoCancellerEnforceHighPass);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("echo-canceller", "automatic-gain-control", EnableAGC);

  UPDATE_PROPERTY_INSIDE_SUBSECTION("noise-suppression", "enable", EnableNoiseSuppression);
  UPDATE_ENUM_LIKE_PROPERTY_INSIDE_SUBSECTION("noise-suppression", "level", NoiseSuppressionLevel);

  UPDATE_PROPERTY_INSIDE_SUBSECTION("high-pass", "enable", EnableHighPassFilter);
  UPDATE_PROPERTY_INSIDE_SUBSECTION("high-pass", "full-band", HighPassFilterFullBand);
}
