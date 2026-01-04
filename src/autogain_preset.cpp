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

#include "autogain_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_autogain.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

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

  json[section][instance_name]["reference"] =
      settings->defaultReferenceLabelsValue()[settings->reference()].toStdString();

  json[section][instance_name]["force-silence"] = settings->forceSilence();
}

void AutoGainPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("target", Target);
  UPDATE_PROPERTY("silence-threshold", SilenceThreshold);
  UPDATE_PROPERTY("maximum-history", MaximumHistory);
  UPDATE_PROPERTY("force-silence", ForceSilence);

  UPDATE_ENUM_LIKE_PROPERTY("reference", Reference);
}
