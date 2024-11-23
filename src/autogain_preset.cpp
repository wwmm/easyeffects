/*
 *  Copyright © 2017-2024 Wellington Wallace
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

  // json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  // json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  // json[section][instance_name]["target"] = g_settings_get_double(settings, "target");

  // json[section][instance_name]["silence-threshold"] = g_settings_get_double(settings, "silence-threshold");

  // json[section][instance_name]["maximum-history"] = g_settings_get_int(settings, "maximum-history");

  // json[section][instance_name]["reference"] = util::gsettings_get_string(settings, "reference");
}

void AutoGainPreset::load(const nlohmann::json& json) {
  settings->setBypass(json.value("bypass", settings->defaultBypassValue()));

  // update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  // update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  // update_key<double>(json.at(section).at(instance_name), settings, "target", "target");

  // update_key<double>(json.at(section).at(instance_name), settings, "silence-threshold", "silence-threshold");

  // update_key<int>(json.at(section).at(instance_name), settings, "maximum-history", "maximum-history");

  // update_key<gchar*>(json.at(section).at(instance_name), settings, "reference", "reference");
}
