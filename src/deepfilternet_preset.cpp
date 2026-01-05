/**
 * Copyright © 2023-2026 Torge Matthies
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

#include "deepfilternet_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_deepfilternet.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

DeepFilterNetPreset::DeepFilterNetPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::DeepFilterNet>(pipeline_type);
}

void DeepFilterNetPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["attenuation-limit"] = settings->attenuationLimit();

  json[section][instance_name]["min-processing-threshold"] = settings->minProcessingThreshold();

  json[section][instance_name]["max-erb-processing-threshold"] = settings->maxErbProcessingThreshold();

  json[section][instance_name]["max-df-processing-threshold"] = settings->maxDfProcessingThreshold();

  json[section][instance_name]["min-processing-buffer"] = settings->minProcessingBuffer();

  json[section][instance_name]["post-filter-beta"] = settings->postFilterBeta();
}

void DeepFilterNetPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("attenuation-limit", AttenuationLimit);
  UPDATE_PROPERTY("min-processing-threshold", MinProcessingThreshold);
  UPDATE_PROPERTY("max-erb-processing-threshold", MaxErbProcessingThreshold);
  UPDATE_PROPERTY("max-df-processing-threshold", MaxDfProcessingThreshold);
  UPDATE_PROPERTY("min-processing-buffer", MinProcessingBuffer);
  UPDATE_PROPERTY("post-filter-beta", PostFilterBeta);
}
