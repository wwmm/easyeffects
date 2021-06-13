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

#include "multiband_gate_preset.hpp"

MultibandGatePreset::MultibandGatePreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.multibandgate",
                                           "/com/github/wwmm/easyeffects/streaminputs/multibandgate/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.multibandgate",
                                            "/com/github/wwmm/easyeffects/streamoutputs/multibandgate/")) {}

void MultibandGatePreset::save(boost::property_tree::ptree& root,
                               const std::string& section,
                               const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".multiband_gate.input-gain", settings->get_double("input-gain"));

  root.put(section + ".multiband_gate.output-gain", settings->get_double("output-gain"));

  root.put(section + ".multiband_gate.freq0", settings->get_double("freq0"));

  root.put(section + ".multiband_gate.freq1", settings->get_double("freq1"));

  root.put(section + ".multiband_gate.freq2", settings->get_double("freq2"));

  root.put(section + ".multiband_gate.mode", settings->get_string("mode"));

  // sub band

  root.put(section + ".multiband_gate.subband.reduction", settings->get_double("range0"));

  root.put(section + ".multiband_gate.subband.threshold", settings->get_double("threshold0"));

  root.put(section + ".multiband_gate.subband.ratio", settings->get_double("ratio0"));

  root.put(section + ".multiband_gate.subband.attack", settings->get_double("attack0"));

  root.put(section + ".multiband_gate.subband.release", settings->get_double("release0"));

  root.put(section + ".multiband_gate.subband.makeup", settings->get_double("makeup0"));

  root.put(section + ".multiband_gate.subband.knee", settings->get_double("knee0"));

  root.put(section + ".multiband_gate.subband.detection", settings->get_string("detection0"));

  root.put(section + ".multiband_gate.subband.bypass", settings->get_boolean("bypass0"));

  root.put(section + ".multiband_gate.subband.solo", settings->get_boolean("solo0"));

  // low band

  root.put(section + ".multiband_gate.lowband.reduction", settings->get_double("range1"));

  root.put(section + ".multiband_gate.lowband.threshold", settings->get_double("threshold1"));

  root.put(section + ".multiband_gate.lowband.ratio", settings->get_double("ratio1"));

  root.put(section + ".multiband_gate.lowband.attack", settings->get_double("attack1"));

  root.put(section + ".multiband_gate.lowband.release", settings->get_double("release1"));

  root.put(section + ".multiband_gate.lowband.makeup", settings->get_double("makeup1"));

  root.put(section + ".multiband_gate.lowband.knee", settings->get_double("knee1"));

  root.put(section + ".multiband_gate.lowband.detection", settings->get_string("detection1"));

  root.put(section + ".multiband_gate.lowband.bypass", settings->get_boolean("bypass1"));

  root.put(section + ".multiband_gate.lowband.solo", settings->get_boolean("solo1"));

  // mid band

  root.put(section + ".multiband_gate.midband.reduction", settings->get_double("range2"));

  root.put(section + ".multiband_gate.midband.threshold", settings->get_double("threshold2"));

  root.put(section + ".multiband_gate.midband.ratio", settings->get_double("ratio2"));

  root.put(section + ".multiband_gate.midband.attack", settings->get_double("attack2"));

  root.put(section + ".multiband_gate.midband.release", settings->get_double("release2"));

  root.put(section + ".multiband_gate.midband.makeup", settings->get_double("makeup2"));

  root.put(section + ".multiband_gate.midband.knee", settings->get_double("knee2"));

  root.put(section + ".multiband_gate.midband.detection", settings->get_string("detection2"));

  root.put(section + ".multiband_gate.midband.bypass", settings->get_boolean("bypass2"));

  root.put(section + ".multiband_gate.midband.solo", settings->get_boolean("solo2"));

  // high band

  root.put(section + ".multiband_gate.highband.reduction", settings->get_double("range3"));

  root.put(section + ".multiband_gate.highband.threshold", settings->get_double("threshold3"));

  root.put(section + ".multiband_gate.highband.ratio", settings->get_double("ratio3"));

  root.put(section + ".multiband_gate.highband.attack", settings->get_double("attack3"));

  root.put(section + ".multiband_gate.highband.release", settings->get_double("release3"));

  root.put(section + ".multiband_gate.highband.makeup", settings->get_double("makeup3"));

  root.put(section + ".multiband_gate.highband.knee", settings->get_double("knee3"));

  root.put(section + ".multiband_gate.highband.detection", settings->get_string("detection3"));

  root.put(section + ".multiband_gate.highband.bypass", settings->get_boolean("bypass3"));

  root.put(section + ".multiband_gate.highband.solo", settings->get_boolean("solo3"));
}

void MultibandGatePreset::load(const nlohmann::json& json,
                               const std::string& section,
                               const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("multiband_gate"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "freq0", "freq0");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "freq1", "freq1");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "freq2", "freq2");

  update_string_key(json.at(section).at("multiband_gate"), settings, "mode", "mode");

  // sub band

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "range0", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "threshold0", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "ratio0", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "attack0", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "release0", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "makeup0", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "knee0", "knee");

  update_string_key(json.at(section).at("multiband_gate").at("subband"), settings, "detection0", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("subband"), settings, "bypass0", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("subband"), settings, "solo0", "solo");

  // low band

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "range1", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "threshold1", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "ratio1", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "attack1", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "release1", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "makeup1", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "knee1", "knee");

  update_string_key(json.at(section).at("multiband_gate").at("lowband"), settings, "detection1", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("lowband"), settings, "bypass1", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("lowband"), settings, "solo1", "solo");

  // mid band

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "range2", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "threshold2", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "ratio2", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "attack2", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "release2", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "makeup2", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "knee2", "knee");

  update_string_key(json.at(section).at("multiband_gate").at("midband"), settings, "detection2", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("midband"), settings, "bypass2", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("midband"), settings, "solo2", "solo");

  // high band

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "range3", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "threshold3", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "ratio3", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "attack3", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "release3", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "makeup3", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "knee3", "knee");

  update_string_key(json.at(section).at("multiband_gate").at("highband"), settings, "detection3", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("highband"), settings, "bypass3", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("highband"), settings, "solo3", "solo");
}

void MultibandGatePreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void MultibandGatePreset::read(PresetType preset_type, const nlohmann::json& json) {
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
