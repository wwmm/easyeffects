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

#include "crystalizer_preset.hpp"

CrystalizerPreset::CrystalizerPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.crystalizer",
                                           "/com/github/wwmm/easyeffects/streaminputs/crystalizer/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.crystalizer",
                                            "/com/github/wwmm/easyeffects/streamoutputs/crystalizer/")) {}

void CrystalizerPreset::save(boost::property_tree::ptree& root,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".crystalizer.input-gain", settings->get_double("input-gain"));

  root.put(section + ".crystalizer.output-gain", settings->get_double("output-gain"));

  for (int n = 0; n < 13; n++) {
    root.put(section + ".crystalizer.band" + std::to_string(n) + ".intensity",
             settings->get_double("intensity-band" + std::to_string(n)));

    root.put(section + ".crystalizer.band" + std::to_string(n) + ".mute",
             settings->get_boolean("mute-band" + std::to_string(n)));

    root.put(section + ".crystalizer.band" + std::to_string(n) + ".bypass",
             settings->get_boolean("bypass-band" + std::to_string(n)));
  }
}

void CrystalizerPreset::load(const nlohmann::json& json,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("crystalizer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("crystalizer"), settings, "output-gain", "output-gain");

  for (int n = 0; n < 13; n++) {
    update_key<double>(json.at(section).at("crystalizer")["band" + std::to_string(n)], settings,
                       "intensity-band" + std::to_string(n), "intensity");

    update_key<bool>(json.at(section).at("crystalizer")["band" + std::to_string(n)], settings,
                     "mute-band" + std::to_string(n), "mute");

    update_key<bool>(json.at(section).at("crystalizer")["band" + std::to_string(n)], settings,
                     "bypass-band" + std::to_string(n), "bypass");
  }
}

void CrystalizerPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void CrystalizerPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {}

void CrystalizerPreset::read(PresetType preset_type, const nlohmann::json& json) {
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
