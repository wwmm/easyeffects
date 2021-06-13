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

PitchPreset::PitchPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.pitch",
                                           "/com/github/wwmm/easyeffects/streaminputs/pitch/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.pitch",
                                            "/com/github/wwmm/easyeffects/streamoutputs/pitch/")) {}

void PitchPreset::save(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".pitch.input-gain", settings->get_double("input-gain"));

  root.put(section + ".pitch.output-gain", settings->get_double("output-gain"));

  root.put(section + ".pitch.cents", settings->get_int("cents"));

  root.put(section + ".pitch.semitones", settings->get_int("semitones"));

  root.put(section + ".pitch.octaves", settings->get_int("octaves"));

  root.put(section + ".pitch.crispness", settings->get_int("crispness"));

  root.put(section + ".pitch.formant-preserving", settings->get_boolean("formant-preserving"));

  root.put(section + ".pitch.faster", settings->get_boolean("faster"));
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

void PitchPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void PitchPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {}

void PitchPreset::read(PresetType preset_type, const nlohmann::json& json) {
  try {
    switch (preset_type) {
      case PresetType::output:
        load(json, "output", output_settings);
        break;
      case PresetType::input:
        load(json, "input", input_settings);
        break;
    }
  } catch (const nlohmann::json::exception& e) {
    util::warning(e.what());
  }
}