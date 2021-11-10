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

#include "exciter_preset.hpp"

ExciterPreset::ExciterPreset() {
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.exciter",
                                            "/com/github/wwmm/easyeffects/streaminputs/exciter/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.exciter",
                                             "/com/github/wwmm/easyeffects/streamoutputs/exciter/");
}

void ExciterPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["exciter"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["exciter"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["exciter"]["amount"] = g_settings_get_double(settings, "amount");

  json[section]["exciter"]["harmonics"] = g_settings_get_double(settings, "harmonics");

  json[section]["exciter"]["scope"] = g_settings_get_double(settings, "scope");

  json[section]["exciter"]["ceil"] = g_settings_get_double(settings, "ceil");

  json[section]["exciter"]["blend"] = g_settings_get_double(settings, "blend");

  json[section]["exciter"]["ceil-active"] = g_settings_get_boolean(settings, "ceil-active");
}

void ExciterPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("exciter"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("exciter"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("exciter"), settings, "amount", "amount");

  update_key<double>(json.at(section).at("exciter"), settings, "harmonics", "harmonics");

  update_key<double>(json.at(section).at("exciter"), settings, "scope", "scope");

  update_key<double>(json.at(section).at("exciter"), settings, "ceil", "ceil");

  update_key<double>(json.at(section).at("exciter"), settings, "blend", "blend");

  update_key<bool>(json.at(section).at("exciter"), settings, "ceil-active", "ceil-active");
}
