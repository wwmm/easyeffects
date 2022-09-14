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

#include "stereo_tools_preset.hpp"

StereoToolsPreset::StereoToolsPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::stereo_tools::id,
                       tags::schema::stereo_tools::input_path,
                       tags::schema::stereo_tools::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::stereo_tools).append("#").append(util::to_string(index));
}

void StereoToolsPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["balance-in"] = g_settings_get_double(settings, "balance-in");

  json[section][instance_name]["balance-out"] = g_settings_get_double(settings, "balance-out");

  json[section][instance_name]["softclip"] = g_settings_get_boolean(settings, "softclip") != 0;

  json[section][instance_name]["mutel"] = g_settings_get_boolean(settings, "mutel") != 0;

  json[section][instance_name]["muter"] = g_settings_get_boolean(settings, "muter") != 0;

  json[section][instance_name]["phasel"] = g_settings_get_boolean(settings, "phasel") != 0;

  json[section][instance_name]["phaser"] = g_settings_get_boolean(settings, "phaser") != 0;

  json[section][instance_name]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section][instance_name]["side-level"] = g_settings_get_double(settings, "slev");

  json[section][instance_name]["side-balance"] = g_settings_get_double(settings, "sbal");

  json[section][instance_name]["middle-level"] = g_settings_get_double(settings, "mlev");

  json[section][instance_name]["middle-panorama"] = g_settings_get_double(settings, "mpan");

  json[section][instance_name]["stereo-base"] = g_settings_get_double(settings, "stereo-base");

  json[section][instance_name]["delay"] = g_settings_get_double(settings, "delay");

  json[section][instance_name]["sc-level"] = g_settings_get_double(settings, "sc-level");

  json[section][instance_name]["stereo-phase"] = g_settings_get_double(settings, "stereo-phase");
}

void StereoToolsPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "balance-in", "balance-in");

  update_key<double>(json.at(section).at(instance_name), settings, "balance-out", "balance-out");

  update_key<bool>(json.at(section).at(instance_name), settings, "softclip", "softclip");

  update_key<bool>(json.at(section).at(instance_name), settings, "mutel", "mutel");

  update_key<bool>(json.at(section).at(instance_name), settings, "muter", "muter");

  update_key<bool>(json.at(section).at(instance_name), settings, "phasel", "phasel");

  update_key<bool>(json.at(section).at(instance_name), settings, "phaser", "phaser");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "mode", "mode");

  update_key<double>(json.at(section).at(instance_name), settings, "slev", "side-level");

  update_key<double>(json.at(section).at(instance_name), settings, "sbal", "side-balance");

  update_key<double>(json.at(section).at(instance_name), settings, "mlev", "middle-level");

  update_key<double>(json.at(section).at(instance_name), settings, "mpan", "middle-panorama");

  update_key<double>(json.at(section).at(instance_name), settings, "stereo-base", "stereo-base");

  update_key<double>(json.at(section).at(instance_name), settings, "delay", "delay");

  update_key<double>(json.at(section).at(instance_name), settings, "sc-level", "sc-level");

  update_key<double>(json.at(section).at(instance_name), settings, "stereo-phase", "stereo-phase");
}
