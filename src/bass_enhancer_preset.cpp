/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include "bass_enhancer_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_bass_enhancer.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"

BassEnhancerPreset::BassEnhancerPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::BassEnhancer>(pipeline_type);
}

void BassEnhancerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["amount"] = settings->amount();

  json[section][instance_name]["harmonics"] = settings->harmonics();

  json[section][instance_name]["scope"] = settings->scope();

  json[section][instance_name]["floor"] = settings->floor();

  json[section][instance_name]["blend"] = settings->blend();

  json[section][instance_name]["floor-active"] = settings->floorActive();
}

void BassEnhancerPreset::load(const nlohmann::json& json) {
  settings->setBypass(json.at(section).at(instance_name).value("bypass", settings->defaultBypassValue()));

  settings->setInputGain(json.at(section).at(instance_name).value("input-gain", settings->defaultInputGainValue()));

  settings->setInputGain(json.at(section).at(instance_name).value("output-gain", settings->defaultOutputGainValue()));

  settings->setAmount(json.at(section).at(instance_name).value("amount", settings->defaultAmountValue()));

  settings->setHarmonics(json.at(section).at(instance_name).value("harmonics", settings->defaultHarmonicsValue()));

  settings->setScope(json.at(section).at(instance_name).value("scope", settings->defaultScopeValue()));

  settings->setFloor(json.at(section).at(instance_name).value("floor", settings->defaultFloorValue()));

  settings->setBlend(json.at(section).at(instance_name).value("blend", settings->defaultBlendValue()));

  settings->setFloorActive(
      json.at(section).at(instance_name).value("floor-active", settings->defaultFloorActiveValue()));
}
