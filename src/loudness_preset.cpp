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

#include "loudness_preset.hpp"
#include <strings.h>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_loudness.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

LoudnessPreset::LoudnessPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Loudness>(pipeline_type);
}

void LoudnessPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["fft"] = settings->defaultFftLabelsValue()[settings->fft()].toStdString();

  json[section][instance_name]["iir-approximation"] =
      settings->defaultIirApproximationLabelsValue()[settings->iirApproximation()].toStdString();

  json[section][instance_name]["std"] = settings->defaultStdLabelsValue()[settings->std()].toStdString();

  json[section][instance_name]["volume"] = settings->volume();

  json[section][instance_name]["clipping"] = settings->clipping();

  json[section][instance_name]["clipping-range"] = settings->clippingRange();
}

void LoudnessPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("volume", Volume);
  UPDATE_PROPERTY("clipping", Clipping);
  UPDATE_PROPERTY("clipping-range", ClippingRange);

  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
  UPDATE_ENUM_LIKE_PROPERTY("fft", Fft);
  UPDATE_ENUM_LIKE_PROPERTY("iir-approximation", IirApproximation);
  UPDATE_ENUM_LIKE_PROPERTY("std", Std);
}
