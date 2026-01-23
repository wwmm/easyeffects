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

#include "voice_suppressor_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_voice_suppressor.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

VoiceSuppressorPreset::VoiceSuppressorPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::VoiceSuppressor>(pipeline_type);
}

void VoiceSuppressorPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["freq-start"] = settings->freqStart();

  json[section][instance_name]["freq-end"] = settings->freqEnd();

  json[section][instance_name]["correlation"] = settings->correlation();

  json[section][instance_name]["phase-difference"] = settings->phaseDifference();

  json[section][instance_name]["minimum-kurtosis"] = settings->minKurtosis();
}

void VoiceSuppressorPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("freq-start", FreqStart);
  UPDATE_PROPERTY("freq-end", FreqEnd);
  UPDATE_PROPERTY("correlation", Correlation);
  UPDATE_PROPERTY("phase-difference", PhaseDifference);
  UPDATE_PROPERTY("minimum-kurtosis", MinKurtosis);
}
