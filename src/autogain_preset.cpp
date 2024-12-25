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

#include "autogain_preset.hpp"
#include <iostream>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_autogain.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"

AutoGainPreset::AutoGainPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Autogain>(pipeline_type);
}

void AutoGainPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["target"] = settings->target();

  json[section][instance_name]["silence-threshold"] = settings->silenceThreshold();

  json[section][instance_name]["maximum-history"] = settings->maximumHistory();

  json[section][instance_name]["reference"] = settings->reference();
}

void AutoGainPreset::load(const nlohmann::json& json) {
  settings->setBypass(json.at(section).at(instance_name).value("bypass", settings->defaultBypassValue()));

  settings->setInputGain(json.at(section).at(instance_name).value("input-gain", settings->defaultInputGainValue()));

  settings->setOutputGain(json.at(section).at(instance_name).value("output-gain", settings->defaultOutputGainValue()));

  settings->setTarget(json.at(section).at(instance_name).value("target", settings->defaultTargetValue()));

  settings->setSilenceThreshold(
      json.at(section).at(instance_name).value("silence-threshold", settings->defaultSilenceThresholdValue()));

  settings->setMaximumHistory(
      json.at(section).at(instance_name).value("maximum-history", settings->defaultMaximumHistoryValue()));

  // Hum... I have to think about how to deal with enums without requiring changes to the preset format...

  // settings->setReference(json.at(section).at(instance_name).value("reference", settings->defaultReferenceValue()));

  // std::cout << json.at(section).at(instance_name).value("reference", settings->defaultReferenceValue()) << std::endl;
}
