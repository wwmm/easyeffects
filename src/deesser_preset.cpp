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

#include "deesser_preset.hpp"

DeesserPreset::DeesserPreset() {
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.deesser",
                                            "/com/github/wwmm/easyeffects/streaminputs/deesser/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.deesser",
                                             "/com/github/wwmm/easyeffects/streamoutputs/deesser/");
}

void DeesserPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["deesser"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["deesser"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["deesser"]["detection"] = g_settings_get_string(settings, "detection");

  json[section]["deesser"]["mode"] = g_settings_get_string(settings, "mode");

  json[section]["deesser"]["threshold"] = g_settings_get_double(settings, "threshold");

  json[section]["deesser"]["ratio"] = g_settings_get_double(settings, "ratio");

  json[section]["deesser"]["laxity"] = g_settings_get_int(settings, "laxity");

  json[section]["deesser"]["makeup"] = g_settings_get_double(settings, "makeup");

  json[section]["deesser"]["f1-freq"] = g_settings_get_double(settings, "f1-freq");

  json[section]["deesser"]["f2-freq"] = g_settings_get_double(settings, "f2-freq");

  json[section]["deesser"]["f1-level"] = g_settings_get_double(settings, "f1-level");

  json[section]["deesser"]["f2-level"] = g_settings_get_double(settings, "f2-level");

  json[section]["deesser"]["f2-q"] = g_settings_get_double(settings, "f2-q");

  json[section]["deesser"]["sc-listen"] = g_settings_get_boolean(settings, "sc-listen") != 0;
}

void DeesserPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("deesser"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("deesser"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("deesser"), settings, "detection", "detection");

  update_key<gchar*>(json.at(section).at("deesser"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("deesser"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("deesser"), settings, "ratio", "ratio");

  update_key<int>(json.at(section).at("deesser"), settings, "laxity", "laxity");

  update_key<double>(json.at(section).at("deesser"), settings, "makeup", "makeup");

  update_key<double>(json.at(section).at("deesser"), settings, "f1-freq", "f1-freq");

  update_key<double>(json.at(section).at("deesser"), settings, "f2-freq", "f2-freq");

  update_key<double>(json.at(section).at("deesser"), settings, "f1-level", "f1-level");

  update_key<double>(json.at(section).at("deesser"), settings, "f2-level", "f2-level");

  update_key<double>(json.at(section).at("deesser"), settings, "f2-q", "f2-q");

  update_key<bool>(json.at(section).at("deesser"), settings, "sc-listen", "sc-listen");
}
