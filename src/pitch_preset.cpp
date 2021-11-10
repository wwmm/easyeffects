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

#include "pitch_preset.hpp"

PitchPreset::PitchPreset() {
  input_settings =
      g_settings_new_with_path("com.github.wwmm.easyeffects.pitch", "/com/github/wwmm/easyeffects/streaminputs/pitch/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.pitch",
                                             "/com/github/wwmm/easyeffects/streamoutputs/pitch/");
}

void PitchPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["pitch"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["pitch"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["pitch"]["cents"] = g_settings_get_int(settings, "cents");

  json[section]["pitch"]["semitones"] = g_settings_get_int(settings, "semitones");

  json[section]["pitch"]["octaves"] = g_settings_get_int(settings, "octaves");

  json[section]["pitch"]["crispness"] = g_settings_get_int(settings, "crispness");

  json[section]["pitch"]["formant-preserving"] = g_settings_get_boolean(settings, "formant-preserving");

  json[section]["pitch"]["faster"] = g_settings_get_boolean(settings, "faster");
}

void PitchPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("pitch"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("pitch"), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at("pitch"), settings, "cents", "cents");

  update_key<int>(json.at(section).at("pitch"), settings, "semitones", "semitones");

  update_key<int>(json.at(section).at("pitch"), settings, "octaves", "octaves");

  update_key<int>(json.at(section).at("pitch"), settings, "crispness", "crispness");

  update_key<bool>(json.at(section).at("pitch"), settings, "formant-preserving", "formant-preserving");

  update_key<bool>(json.at(section).at("pitch"), settings, "faster", "faster");
}
