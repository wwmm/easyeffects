/*
 *  Copyright © 2017-2020 Wellington Wallace
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
      Gio::Settings::create("com.github.wwmm.easyeffects.pitch", "/com/github/wwmm/easyeffects/streaminputs/pitch/");

  output_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.pitch", "/com/github/wwmm/easyeffects/streamoutputs/pitch/");
}

void PitchPreset::save(nlohmann::json& json, const std::string& section, const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["pitch"]["input-gain"] = settings->get_double("input-gain");

  json[section]["pitch"]["output-gain"] = settings->get_double("output-gain");

  json[section]["pitch"]["cents"] = settings->get_int("cents");

  json[section]["pitch"]["semitones"] = settings->get_int("semitones");

  json[section]["pitch"]["octaves"] = settings->get_int("octaves");

  json[section]["pitch"]["crispness"] = settings->get_int("crispness");

  json[section]["pitch"]["formant-preserving"] = settings->get_boolean("formant-preserving");

  json[section]["pitch"]["faster"] = settings->get_boolean("faster");
}

void PitchPreset::load(const nlohmann::json& json,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("pitch"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("pitch"), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at("pitch"), settings, "cents", "cents");

  update_key<int>(json.at(section).at("pitch"), settings, "semitones", "semitones");

  update_key<int>(json.at(section).at("pitch"), settings, "octaves", "octaves");

  update_key<int>(json.at(section).at("pitch"), settings, "crispness", "crispness");

  update_key<bool>(json.at(section).at("pitch"), settings, "formant-preserving", "formant-preserving");

  update_key<bool>(json.at(section).at("pitch"), settings, "faster", "faster");
}
