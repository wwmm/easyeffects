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

#include "reverb_preset.hpp"

ReverbPreset::ReverbPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::reverb::id,
                       tags::schema::reverb::input_path,
                       tags::schema::reverb::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::reverb).append("#").append(util::to_string(index));
}

void ReverbPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["room-size"] = util::gsettings_get_string(settings, "room-size");

  json[section][instance_name]["decay-time"] = g_settings_get_double(settings, "decay-time");

  json[section][instance_name]["hf-damp"] = g_settings_get_double(settings, "hf-damp");

  json[section][instance_name]["diffusion"] = g_settings_get_double(settings, "diffusion");

  json[section][instance_name]["amount"] = g_settings_get_double(settings, "amount");

  json[section][instance_name]["dry"] = g_settings_get_double(settings, "dry");

  json[section][instance_name]["predelay"] = g_settings_get_double(settings, "predelay");

  json[section][instance_name]["bass-cut"] = g_settings_get_double(settings, "bass-cut");

  json[section][instance_name]["treble-cut"] = g_settings_get_double(settings, "treble-cut");
}

void ReverbPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "room-size", "room-size");

  update_key<double>(json.at(section).at(instance_name), settings, "decay-time", "decay-time");

  update_key<double>(json.at(section).at(instance_name), settings, "hf-damp", "hf-damp");

  update_key<double>(json.at(section).at(instance_name), settings, "diffusion", "diffusion");

  update_key<double>(json.at(section).at(instance_name), settings, "amount", "amount");

  update_key<double>(json.at(section).at(instance_name), settings, "dry", "dry");

  update_key<double>(json.at(section).at(instance_name), settings, "predelay", "predelay");

  update_key<double>(json.at(section).at(instance_name), settings, "bass-cut", "bass-cut");

  update_key<double>(json.at(section).at(instance_name), settings, "treble-cut", "treble-cut");
}
