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

#include "delay_preset.hpp"

DelayPreset::DelayPreset() {
  input_settings = g_settings_new_with_path(tags::schema::delay::id, tags::schema::delay::input_path);

  output_settings = g_settings_new_with_path(tags::schema::delay::id, tags::schema::delay::output_path);
}

void DelayPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["delay"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["delay"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["delay"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["delay"]["time-l"] = g_settings_get_double(settings, "time-l");

  json[section]["delay"]["time-r"] = g_settings_get_double(settings, "time-r");
}

void DelayPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<bool>(json.at(section).at("delay"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("delay"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("delay"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("delay"), settings, "time-l", "time-l");

  update_key<double>(json.at(section).at("delay"), settings, "time-r", "time-r");
}
