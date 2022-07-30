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

#include "filter_preset.hpp"

FilterPreset::FilterPreset(PresetType preset_type, const int& index) : PluginPresetBase(preset_type, index) {
  switch (preset_type) {
    case PresetType::input:
      settings = g_settings_new_with_path(tags::schema::filter::id, tags::schema::filter::input_path);
      break;
    case PresetType::output:
      settings = g_settings_new_with_path(tags::schema::filter::id, tags::schema::filter::output_path);
      break;
  }
}

void FilterPreset::save(nlohmann::json& json) {
  json[section]["filter"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["filter"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["filter"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["filter"]["frequency"] = g_settings_get_double(settings, "frequency");

  json[section]["filter"]["resonance"] = g_settings_get_double(settings, "resonance");

  json[section]["filter"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["filter"]["inertia"] = g_settings_get_double(settings, "inertia");
}

void FilterPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at("filter"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("filter"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("filter"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("filter"), settings, "frequency", "frequency");

  update_key<double>(json.at(section).at("filter"), settings, "resonance", "resonance");

  update_key<gchar*>(json.at(section).at("filter"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("filter"), settings, "inertia", "inertia");
}
