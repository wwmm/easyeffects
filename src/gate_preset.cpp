/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "gate_preset.hpp"

GatePreset::GatePreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.gate",
                                           "/com/github/wwmm/pulseeffects/sourceoutputs/gate/")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.gate",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/gate/")) {}

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

void GatePreset::load(const boost::property_tree::ptree& root,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(root, settings, "input-gain", section + ".gate.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".gate.output-gain");

  update_string_key(root, settings, "detection", section + ".gate.detection");

  update_string_key(root, settings, "stereo-link", section + ".gate.stereo-link");

  update_key<double>(root, settings, "range", section + ".gate.range");

  update_key<double>(root, settings, "attack", section + ".gate.attack");

  update_key<double>(root, settings, "release", section + ".gate.release");

  update_key<double>(root, settings, "threshold", section + ".gate.threshold");

  update_key<double>(root, settings, "ratio", section + ".gate.ratio");

  update_key<double>(root, settings, "knee", section + ".gate.knee");

  update_key<double>(root, settings, "makeup", section + ".gate.makeup");
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

void GatePreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      load(root, "output", output_settings);
      break;
    case PresetType::input:
      load(root, "input", input_settings);
      break;
  }
}
