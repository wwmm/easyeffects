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

#include "crusher_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_crusher.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

CrusherPreset::CrusherPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Crusher>(pipeline_type);
}

void CrusherPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["bit-reduction"] = settings->bitReduction();

  json[section][instance_name]["morph"] = settings->morph();

  json[section][instance_name]["anti-aliasing"] = settings->antiAliasing();

  json[section][instance_name]["sample-reduction"] = settings->sampleReduction();

  json[section][instance_name]["lfo-active"] = settings->lfoActive();

  json[section][instance_name]["lfo-range"] = settings->lfoRange();

  json[section][instance_name]["lfo-rate"] = settings->lfoRate();

  json[section][instance_name]["dc"] = settings->dc();
}

void CrusherPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("bit-reduction", BitReduction);
  UPDATE_PROPERTY("morph", Morph);
  UPDATE_PROPERTY("anti-aliasing", AntiAliasing);
  UPDATE_PROPERTY("sample-reduction", SampleReduction);
  UPDATE_PROPERTY("lfo-active", LfoActive);
  UPDATE_PROPERTY("lfo-range", LfoRange);
  UPDATE_PROPERTY("lfo-rate", LfoRate);
  UPDATE_PROPERTY("dc", Dc);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
}
