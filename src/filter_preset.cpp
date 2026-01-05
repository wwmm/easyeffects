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

#include "filter_preset.hpp"
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "easyeffects_db_filter.h"
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_macros.hpp"

FilterPreset::FilterPreset(PipelineType pipeline_type, const std::string& instance_name)
    : PluginPresetBase(pipeline_type, instance_name) {
  settings = get_db_instance<db::Filter>(pipeline_type);
}

void FilterPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = settings->bypass();

  json[section][instance_name]["input-gain"] = settings->inputGain();

  json[section][instance_name]["output-gain"] = settings->outputGain();

  json[section][instance_name]["frequency"] = settings->frequency();

  json[section][instance_name]["width"] = settings->width();

  json[section][instance_name]["gain"] = settings->gain();

  json[section][instance_name]["quality"] = settings->quality();

  json[section][instance_name]["balance"] = settings->balance();

  json[section][instance_name]["type"] = settings->defaultTypeLabelsValue()[settings->type()].toStdString();

  json[section][instance_name]["mode"] = settings->defaultModeLabelsValue()[settings->mode()].toStdString();

  json[section][instance_name]["equal-mode"] =
      settings->defaultEqualModeLabelsValue()[settings->equalMode()].toStdString();

  json[section][instance_name]["slope"] = settings->defaultSlopeLabelsValue()[settings->slope()].toStdString();
}

void FilterPreset::load(const nlohmann::json& json) {
  UPDATE_PROPERTY("bypass", Bypass);
  UPDATE_PROPERTY("input-gain", InputGain);
  UPDATE_PROPERTY("output-gain", OutputGain);
  UPDATE_PROPERTY("frequency", Frequency);
  UPDATE_PROPERTY("width", Width);
  UPDATE_PROPERTY("gain", Gain);
  UPDATE_PROPERTY("quality", Quality);
  UPDATE_PROPERTY("balance", Balance);

  UPDATE_ENUM_LIKE_PROPERTY("type", Type);
  UPDATE_ENUM_LIKE_PROPERTY("mode", Mode);
  UPDATE_ENUM_LIKE_PROPERTY("equal-mode", EqualMode);
  UPDATE_ENUM_LIKE_PROPERTY("slope", Slope);
}
