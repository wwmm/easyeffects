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

#include "rnnoise_preset.hpp"

RNNoisePreset::RNNoisePreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.rnnoise",
                                            "/com/github/wwmm/easyeffects/streamoutputs/rnnoise/")),
      input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.rnnoise",
                                           "/com/github/wwmm/easyeffects/streaminputs/rnnoise/")) {}

void RNNoisePreset::save(boost::property_tree::ptree& root,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".rnnoise.input-gain", settings->get_double("input-gain"));

  root.put(section + ".rnnoise.output-gain", settings->get_double("output-gain"));

  root.put(section + ".rnnoise.model-path", settings->get_string("model-path"));
}

void RNNoisePreset::load(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("rnnoise"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("rnnoise"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("rnnoise"), settings, "model-path", "model-path");
}

void RNNoisePreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void RNNoisePreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {}

void RNNoisePreset::read(PresetType preset_type, const nlohmann::json& json) {
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
