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

#include "rnnoise_preset.hpp"

RNNoisePreset::RNNoisePreset() {
  input_settings = g_settings_new_with_path(tags::schema::rnnoise::id, tags::schema::rnnoise::input_path);

  output_settings = g_settings_new_with_path(tags::schema::rnnoise::id, tags::schema::rnnoise::output_path);
}

void RNNoisePreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["rnnoise"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["rnnoise"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["rnnoise"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["rnnoise"]["model-path"] = util::gsettings_get_string(settings, "model-path");
}

void RNNoisePreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<bool>(json.at(section).at("rnnoise"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("rnnoise"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("rnnoise"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("rnnoise"), settings, "model-path", "model-path");
}
