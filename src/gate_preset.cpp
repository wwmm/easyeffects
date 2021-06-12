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

#include "gate_preset.hpp"

GatePreset::GatePreset()
    : input_settings(
          Gio::Settings::create("com.github.wwmm.easyeffects.gate", "/com/github/wwmm/easyeffects/streaminputs/gate/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.gate",
                                            "/com/github/wwmm/easyeffects/streamoutputs/gate/")) {}

void GatePreset::save(boost::property_tree::ptree& root,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".gate.input-gain", settings->get_double("input-gain"));

  root.put(section + ".gate.output-gain", settings->get_double("output-gain"));

  root.put(section + ".gate.detection", settings->get_string("detection"));

  root.put(section + ".gate.stereo-link", settings->get_string("stereo-link"));

  root.put(section + ".gate.range", settings->get_double("range"));

  root.put(section + ".gate.attack", settings->get_double("attack"));

  root.put(section + ".gate.release", settings->get_double("release"));

  root.put(section + ".gate.threshold", settings->get_double("threshold"));

  root.put(section + ".gate.ratio", settings->get_double("ratio"));

  root.put(section + ".gate.knee", settings->get_double("knee"));

  root.put(section + ".gate.makeup", settings->get_double("makeup"));
}

void GatePreset::load(const nlohmann::json& json,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("gate"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("gate"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("gate"), settings, "detection", "detection");

  update_string_key(json.at(section).at("gate"), settings, "stereo-link", "stereo-link");

  update_key<double>(json.at(section).at("gate"), settings, "range", "range");

  update_key<double>(json.at(section).at("gate"), settings, "attack", "attack");

  update_key<double>(json.at(section).at("gate"), settings, "release", "release");

  update_key<double>(json.at(section).at("gate"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("gate"), settings, "ratio", "ratio");

  update_key<double>(json.at(section).at("gate"), settings, "knee", "knee");

  update_key<double>(json.at(section).at("gate"), settings, "makeup", "makeup");
}

void GatePreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void GatePreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {}

void GatePreset::read(PresetType preset_type, const nlohmann::json& json) {
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
