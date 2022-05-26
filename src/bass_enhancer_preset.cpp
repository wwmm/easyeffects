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

#include "bass_enhancer_preset.hpp"

BassEnhancerPreset::BassEnhancerPreset() {
  input_settings =
      g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streaminputs/bassenhancer/").c_str());

  output_settings =
      g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streamoutputs/bassenhancer/").c_str());
}

void BassEnhancerPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["bass_enhancer"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["bass_enhancer"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["bass_enhancer"]["amount"] = g_settings_get_double(settings, "amount");

  json[section]["bass_enhancer"]["harmonics"] = g_settings_get_double(settings, "harmonics");

  json[section]["bass_enhancer"]["scope"] = g_settings_get_double(settings, "scope");

  json[section]["bass_enhancer"]["floor"] = g_settings_get_double(settings, "floor");

  json[section]["bass_enhancer"]["blend"] = g_settings_get_double(settings, "blend");

  json[section]["bass_enhancer"]["floor-active"] = g_settings_get_boolean(settings, "floor-active") != 0;
}

void BassEnhancerPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("bass_enhancer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "amount", "amount");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "harmonics", "harmonics");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "scope", "scope");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "floor", "floor");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "blend", "blend");

  update_key<bool>(json.at(section).at("bass_enhancer"), settings, "floor-active", "floor-active");
}
