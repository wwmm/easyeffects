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

void MultibandGatePreset::load(const boost::property_tree::ptree& root,
                               const std::string& section,
                               const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(root, settings, "input-gain", section + ".multiband_gate.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".multiband_gate.output-gain");

  update_key<double>(root, settings, "freq0", section + ".multiband_gate.freq0");

  update_key<double>(root, settings, "freq1", section + ".multiband_gate.freq1");

  update_key<double>(root, settings, "freq2", section + ".multiband_gate.freq2");

  update_string_key(root, settings, "mode", section + ".multiband_gate.mode");

  // sub band

  update_key<double>(root, settings, "range0", section + ".multiband_gate.subband.reduction");

  update_key<double>(root, settings, "threshold0", section + ".multiband_gate.subband.threshold");

  update_key<double>(root, settings, "ratio0", section + ".multiband_gate.subband.ratio");

  update_key<double>(root, settings, "attack0", section + ".multiband_gate.subband.attack");

  update_key<double>(root, settings, "release0", section + ".multiband_gate.subband.release");

  update_key<double>(root, settings, "makeup0", section + ".multiband_gate.subband.makeup");

  update_key<double>(root, settings, "knee0", section + ".multiband_gate.subband.knee");

  update_string_key(root, settings, "detection0", section + ".multiband_gate.subband.detection");

  update_key<bool>(root, settings, "bypass0", section + ".multiband_gate.subband.bypass");

  update_key<bool>(root, settings, "solo0", section + ".multiband_gate.subband.solo");

  // low band

  update_key<double>(root, settings, "range1", section + ".multiband_gate.lowband.reduction");

  update_key<double>(root, settings, "threshold1", section + ".multiband_gate.lowband.threshold");

  update_key<double>(root, settings, "ratio1", section + ".multiband_gate.lowband.ratio");

  update_key<double>(root, settings, "attack1", section + ".multiband_gate.lowband.attack");

  update_key<double>(root, settings, "release1", section + ".multiband_gate.lowband.release");

  update_key<double>(root, settings, "makeup1", section + ".multiband_gate.lowband.makeup");

  update_key<double>(root, settings, "knee1", section + ".multiband_gate.lowband.knee");

  update_string_key(root, settings, "detection1", section + ".multiband_gate.lowband.detection");

  update_key<bool>(root, settings, "bypass1", section + ".multiband_gate.lowband.bypass");

  update_key<bool>(root, settings, "solo1", section + ".multiband_gate.lowband.solo");

  // mid band

  update_key<double>(root, settings, "range2", section + ".multiband_gate.midband.reduction");

  update_key<double>(root, settings, "threshold2", section + ".multiband_gate.midband.threshold");

  update_key<double>(root, settings, "ratio2", section + ".multiband_gate.midband.ratio");

  update_key<double>(root, settings, "attack2", section + ".multiband_gate.midband.attack");

  update_key<double>(root, settings, "release2", section + ".multiband_gate.midband.release");

  update_key<double>(root, settings, "makeup2", section + ".multiband_gate.midband.makeup");

  update_key<double>(root, settings, "knee2", section + ".multiband_gate.midband.knee");

  update_string_key(root, settings, "detection2", section + ".multiband_gate.midband.detection");

  update_key<bool>(root, settings, "bypass2", section + ".multiband_gate.midband.bypass");

  update_key<bool>(root, settings, "solo2", section + ".multiband_gate.midband.solo");

  // high band

  update_key<double>(root, settings, "range3", section + ".multiband_gate.highband.reduction");

  update_key<double>(root, settings, "threshold3", section + ".multiband_gate.highband.threshold");

  update_key<double>(root, settings, "ratio3", section + ".multiband_gate.highband.ratio");

  update_key<double>(root, settings, "attack3", section + ".multiband_gate.highband.attack");

  update_key<double>(root, settings, "release3", section + ".multiband_gate.highband.release");

  update_key<double>(root, settings, "makeup3", section + ".multiband_gate.highband.makeup");

  update_key<double>(root, settings, "knee3", section + ".multiband_gate.highband.knee");

  update_string_key(root, settings, "detection3", section + ".multiband_gate.highband.detection");

  update_key<bool>(root, settings, "bypass3", section + ".multiband_gate.highband.bypass");

  update_key<bool>(root, settings, "solo3", section + ".multiband_gate.highband.solo");
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

void MultibandGatePreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      load(root, "output", output_settings);
      break;
    case PresetType::input:
      load(root, "input", input_settings);
      break;
  }
}
