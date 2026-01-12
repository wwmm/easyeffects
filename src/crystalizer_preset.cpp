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

#include "crystalizer_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_crystalizer.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"
#include "util.hpp"

// NOLINTNEXTLINE
#define SAVE_BAND(index)                                                                 \
  {                                                                                      \
    const auto bandn = "band" + util::to_string(index);                                  \
    json[section][instance_name][bandn]["intensity"] = settings->intensityBand##index(); \
    json[section][instance_name][bandn]["mute"] = settings->muteBand##index();           \
    json[section][instance_name][bandn]["bypass"] = settings->bypassBand##index();       \
  }

// NOLINTNEXTLINE
#define LOAD_BAND(index)                                                                                               \
  {                                                                                                                    \
    const auto bandn = "band" + util::to_string(index);                                                                \
    settings->setIntensityBand##index(                                                                                 \
        json.at(section).at(instance_name)[bandn].value("intensity", settings->defaultIntensityBand##index##Value())); \
    settings->setMuteBand##index(                                                                                      \
        json.at(section).at(instance_name)[bandn].value("mute", settings->defaultMuteBand##index##Value()));           \
    settings->setBypassBand##index(                                                                                    \
        json.at(section).at(instance_name)[bandn].value("bypass", settings->defaultBypassBand##index##Value()));       \
  }

CrystalizerPreset::CrystalizerPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Crystalizer>(pipeline_type);
}

void CrystalizerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["adaptive-intensity"] = settings->adaptiveIntensity();

  json[section][instance_name]["fixed-quantum"] = settings->useFixedQuantum();

  json[section][instance_name]["oversampling"] = settings->oversampling();

  json[section][instance_name]["oversampling-quality"] = settings->oversamplingQuality();

  SAVE_BAND(0);
  SAVE_BAND(1);
  SAVE_BAND(2);
  SAVE_BAND(3);
  SAVE_BAND(4);
  SAVE_BAND(5);
  SAVE_BAND(6);
  SAVE_BAND(7);
  SAVE_BAND(8);
  SAVE_BAND(9);
  SAVE_BAND(10);
  SAVE_BAND(11);
  SAVE_BAND(12);
}

void CrystalizerPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("adaptive-intensity", AdaptiveIntensity);
  UPDATE_PROPERTY("fixed-quantum", UseFixedQuantum);
  UPDATE_PROPERTY("oversampling", Oversampling);
  UPDATE_PROPERTY("oversampling-quality", OversamplingQuality);

  LOAD_BAND(0);
  LOAD_BAND(1);
  LOAD_BAND(2);
  LOAD_BAND(3);
  LOAD_BAND(4);
  LOAD_BAND(5);
  LOAD_BAND(6);
  LOAD_BAND(7);
  LOAD_BAND(8);
  LOAD_BAND(9);
  LOAD_BAND(10);
  LOAD_BAND(11);
  LOAD_BAND(12);
}
