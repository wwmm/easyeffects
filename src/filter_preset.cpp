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

#include "filter_preset.hpp"
#include <gio/gio.h>
#include <glib.h>
#include <nlohmann/json_fwd.hpp>
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"
#include "tags_plugin_name.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

FilterPreset::FilterPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::filter::id,
                       tags::schema::filter::input_path,
                       tags::schema::filter::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::filter).append("#").append(util::to_string(index));
}

void FilterPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["frequency"] = g_settings_get_double(settings, "frequency");

  json[section][instance_name]["width"] = g_settings_get_double(settings, "width");

  json[section][instance_name]["gain"] = g_settings_get_double(settings, "gain");

  json[section][instance_name]["quality"] = g_settings_get_double(settings, "quality");

  json[section][instance_name]["balance"] = g_settings_get_double(settings, "balance");

  json[section][instance_name]["type"] = util::gsettings_get_string(settings, "type");

  json[section][instance_name]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section][instance_name]["equal-mode"] = util::gsettings_get_string(settings, "equal-mode");

  json[section][instance_name]["slope"] = util::gsettings_get_string(settings, "slope");
}

void FilterPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "frequency", "frequency");

  update_key<double>(json.at(section).at(instance_name), settings, "width", "width");

  update_key<double>(json.at(section).at(instance_name), settings, "gain", "gain");

  update_key<double>(json.at(section).at(instance_name), settings, "quality", "quality");

  update_key<double>(json.at(section).at(instance_name), settings, "balance", "balance");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "type", "type");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "mode", "mode");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "equal-mode", "equal-mode");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "slope", "slope");
}
