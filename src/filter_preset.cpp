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

FilterPreset::FilterPreset() {
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.filter",
                                            "/com/github/wwmm/easyeffects/streaminputs/filter/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.filter",
                                             "/com/github/wwmm/easyeffects/streamoutputs/filter/");
}

void FilterPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["filter"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["filter"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["filter"]["frequency"] = g_settings_get_double(settings, "frequency");

  json[section]["filter"]["resonance"] = g_settings_get_double(settings, "resonance");

  json[section]["filter"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["filter"]["inertia"] = g_settings_get_double(settings, "inertia");
}

void FilterPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("filter"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("filter"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("filter"), settings, "frequency", "frequency");

  update_key<double>(json.at(section).at("filter"), settings, "resonance", "resonance");

  update_key<gchar*>(json.at(section).at("filter"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("filter"), settings, "inertia", "inertia");
}
