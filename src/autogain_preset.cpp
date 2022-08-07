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

#include "autogain_preset.hpp"

AutoGainPreset::AutoGainPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::autogain::id,
                       tags::schema::autogain::input_path,
                       tags::schema::autogain::output_path,
                       preset_type,
                       index) {}

void AutoGainPreset::save(nlohmann::json& json) {
  json[section]["autogain"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["autogain"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["autogain"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["autogain"]["target"] = g_settings_get_double(settings, "target");

  json[section]["autogain"]["maximum-history"] = g_settings_get_int(settings, "maximum-history");

  json[section]["autogain"]["reference"] = util::gsettings_get_string(settings, "reference");
}

void AutoGainPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at("autogain"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("autogain"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("autogain"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("autogain"), settings, "target", "target");

  update_key<int>(json.at(section).at("autogain"), settings, "maximum-history", "maximum-history");

  update_key<gchar*>(json.at(section).at("autogain"), settings, "reference", "reference");
}
