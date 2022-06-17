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

#include "crossfeed_preset.hpp"

CrossfeedPreset::CrossfeedPreset() {
  input_settings = g_settings_new_with_path(tags::schema::crossfeed::id, tags::schema::crossfeed::input_path);

  output_settings = g_settings_new_with_path(tags::schema::crossfeed::id, tags::schema::crossfeed::output_path);
}

void CrossfeedPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["crossfeed"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["crossfeed"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["crossfeed"]["fcut"] = g_settings_get_int(settings, "fcut");

  json[section]["crossfeed"]["feed"] = g_settings_get_double(settings, "feed");
}

void CrossfeedPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("crossfeed"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("crossfeed"), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at("crossfeed"), settings, "fcut", "fcut");

  update_key<double>(json.at(section).at("crossfeed"), settings, "feed", "feed");
}
