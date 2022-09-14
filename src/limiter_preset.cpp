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

#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::limiter::id,
                       tags::schema::limiter::input_path,
                       tags::schema::limiter::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::limiter).append("#").append(util::to_string(index));
}

void LimiterPreset::save(nlohmann::json& json) {
  json[section][instance_name]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section][instance_name]["oversampling"] = util::gsettings_get_string(settings, "oversampling");

  json[section][instance_name]["dithering"] = util::gsettings_get_string(settings, "dithering");

  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["lookahead"] = g_settings_get_double(settings, "lookahead");

  json[section][instance_name]["attack"] = g_settings_get_double(settings, "attack");

  json[section][instance_name]["release"] = g_settings_get_double(settings, "release");

  json[section][instance_name]["threshold"] = g_settings_get_double(settings, "threshold");

  json[section][instance_name]["sidechain-preamp"] = g_settings_get_double(settings, "sidechain-preamp");

  json[section][instance_name]["stereo-link"] = g_settings_get_double(settings, "stereo-link");

  json[section][instance_name]["alr-attack"] = g_settings_get_double(settings, "alr-attack");

  json[section][instance_name]["alr-release"] = g_settings_get_double(settings, "alr-release");

  json[section][instance_name]["alr-knee"] = g_settings_get_double(settings, "alr-knee");

  json[section][instance_name]["alr"] = g_settings_get_boolean(settings, "alr") != 0;

  json[section][instance_name]["gain-boost"] = g_settings_get_boolean(settings, "gain-boost") != 0;

  json[section][instance_name]["external-sidechain"] = g_settings_get_boolean(settings, "external-sidechain") != 0;
}

void LimiterPreset::load(const nlohmann::json& json) {
  update_key<gchar*>(json.at(section).at(instance_name), settings, "mode", "mode");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "oversampling", "oversampling");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "dithering", "dithering");

  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "lookahead", "lookahead");

  update_key<double>(json.at(section).at(instance_name), settings, "attack", "attack");

  update_key<double>(json.at(section).at(instance_name), settings, "release", "release");

  update_key<double>(json.at(section).at(instance_name), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at(instance_name), settings, "sidechain-preamp", "sidechain-preamp");

  update_key<double>(json.at(section).at(instance_name), settings, "stereo-link", "stereo-link");

  update_key<double>(json.at(section).at(instance_name), settings, "alr-attack", "alr-attack");

  update_key<double>(json.at(section).at(instance_name), settings, "alr-release", "alr-release");

  update_key<double>(json.at(section).at(instance_name), settings, "alr-knee", "alr-knee");

  update_key<bool>(json.at(section).at(instance_name), settings, "alr", "alr");

  update_key<bool>(json.at(section).at(instance_name), settings, "gain-boost", "gain-boost");

  update_key<bool>(json.at(section).at(instance_name), settings, "external-sidechain", "external-sidechain");
}
