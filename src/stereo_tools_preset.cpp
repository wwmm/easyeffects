/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stereo_tools_preset.hpp"

StereoToolsPreset::StereoToolsPreset() {
  input_settings = g_settings_new_with_path(tags::schema::stereo_tools::id, tags::schema::stereo_tools::input_path);

  output_settings = g_settings_new_with_path(tags::schema::stereo_tools::id, tags::schema::stereo_tools::output_path);
}

void StereoToolsPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["stereo_tools"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["stereo_tools"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["stereo_tools"]["balance-in"] = g_settings_get_double(settings, "balance-in");

  json[section]["stereo_tools"]["balance-out"] = g_settings_get_double(settings, "balance-out");

  json[section]["stereo_tools"]["softclip"] = g_settings_get_boolean(settings, "softclip") != 0;

  json[section]["stereo_tools"]["mutel"] = g_settings_get_boolean(settings, "mutel") != 0;

  json[section]["stereo_tools"]["muter"] = g_settings_get_boolean(settings, "muter") != 0;

  json[section]["stereo_tools"]["phasel"] = g_settings_get_boolean(settings, "phasel") != 0;

  json[section]["stereo_tools"]["phaser"] = g_settings_get_boolean(settings, "phaser") != 0;

  json[section]["stereo_tools"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["stereo_tools"]["side-level"] = g_settings_get_double(settings, "slev");

  json[section]["stereo_tools"]["side-balance"] = g_settings_get_double(settings, "sbal");

  json[section]["stereo_tools"]["middle-level"] = g_settings_get_double(settings, "mlev");

  json[section]["stereo_tools"]["middle-panorama"] = g_settings_get_double(settings, "mpan");

  json[section]["stereo_tools"]["stereo-base"] = g_settings_get_double(settings, "stereo-base");

  json[section]["stereo_tools"]["delay"] = g_settings_get_double(settings, "delay");

  json[section]["stereo_tools"]["sc-level"] = g_settings_get_double(settings, "sc-level");

  json[section]["stereo_tools"]["stereo-phase"] = g_settings_get_double(settings, "stereo-phase");
}

void StereoToolsPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("stereo_tools"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "balance-in", "balance-in");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "balance-out", "balance-out");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "softclip", "softclip");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "mutel", "mutel");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "muter", "muter");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "phasel", "phasel");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "phaser", "phaser");

  update_key<gchar*>(json.at(section).at("stereo_tools"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "slev", "side-level");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "sbal", "side-balance");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "mlev", "middle-level");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "mpan", "middle-panorama");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "stereo-base", "stereo-base");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "delay", "delay");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "sc-level", "sc-level");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "stereo-phase", "stereo-phase");
}
