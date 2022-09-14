/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include "delay_preset.hpp"

DelayPreset::DelayPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::delay::id,
                       tags::schema::delay::input_path,
                       tags::schema::delay::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::delay).append("#").append(util::to_string(index));
}

void DelayPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["time-l"] = g_settings_get_double(settings, "time-l");

  json[section][instance_name]["time-r"] = g_settings_get_double(settings, "time-r");

  json[section][instance_name]["dry-l"] = g_settings_get_double(settings, "dry-l");

  json[section][instance_name]["dry-r"] = g_settings_get_double(settings, "dry-r");

  json[section][instance_name]["wet-l"] = g_settings_get_double(settings, "wet-l");

  json[section][instance_name]["wet-r"] = g_settings_get_double(settings, "wet-r");
}

void DelayPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "time-l", "time-l");

  update_key<double>(json.at(section).at(instance_name), settings, "time-r", "time-r");

  update_key<double>(json.at(section).at(instance_name), settings, "dry-l", "dry-l");

  update_key<double>(json.at(section).at(instance_name), settings, "dry-r", "dry-r");

  update_key<double>(json.at(section).at(instance_name), settings, "wet-l", "wet-l");

  update_key<double>(json.at(section).at(instance_name), settings, "wet-r", "wet-r");
}
