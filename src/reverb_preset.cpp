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

#include "reverb_preset.hpp"

ReverbPreset::ReverbPreset() {
  input_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streaminputs/reverb/").c_str());

  output_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streamoutputs/reverb/").c_str());
}

void ReverbPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["reverb"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["reverb"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["reverb"]["room-size"] = util::gsettings_get_string(settings, "room-size");

  json[section]["reverb"]["decay-time"] = g_settings_get_double(settings, "decay-time");

  json[section]["reverb"]["hf-damp"] = g_settings_get_double(settings, "hf-damp");

  json[section]["reverb"]["diffusion"] = g_settings_get_double(settings, "diffusion");

  json[section]["reverb"]["amount"] = g_settings_get_double(settings, "amount");

  json[section]["reverb"]["dry"] = g_settings_get_double(settings, "dry");

  json[section]["reverb"]["predelay"] = g_settings_get_double(settings, "predelay");

  json[section]["reverb"]["bass-cut"] = g_settings_get_double(settings, "bass-cut");

  json[section]["reverb"]["treble-cut"] = g_settings_get_double(settings, "treble-cut");
}

void ReverbPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("reverb"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("reverb"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("reverb"), settings, "room-size", "room-size");

  update_key<double>(json.at(section).at("reverb"), settings, "decay-time", "decay-time");

  update_key<double>(json.at(section).at("reverb"), settings, "hf-damp", "hf-damp");

  update_key<double>(json.at(section).at("reverb"), settings, "diffusion", "diffusion");

  update_key<double>(json.at(section).at("reverb"), settings, "amount", "amount");

  update_key<double>(json.at(section).at("reverb"), settings, "dry", "dry");

  update_key<double>(json.at(section).at("reverb"), settings, "predelay", "predelay");

  update_key<double>(json.at(section).at("reverb"), settings, "bass-cut", "bass-cut");

  update_key<double>(json.at(section).at("reverb"), settings, "treble-cut", "treble-cut");
}
