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

#include "reverb_preset.hpp"

ReverbPreset::ReverbPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.reverb",
                                           "/com/github/wwmm/pulseeffects/sourceoutputs/reverb/")),
      output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.reverb",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/reverb/")) {}

void ReverbPreset::save(boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".reverb.input-gain", settings->get_double("input-gain"));

  root.put(section + ".reverb.output-gain", settings->get_double("output-gain"));

  root.put(section + ".reverb.room-size", settings->get_string("room-size"));

  root.put(section + ".reverb.decay-time", settings->get_double("decay-time"));

  root.put(section + ".reverb.hf-damp", settings->get_double("hf-damp"));

  root.put(section + ".reverb.diffusion", settings->get_double("diffusion"));

  root.put(section + ".reverb.amount", settings->get_double("amount"));

  root.put(section + ".reverb.dry", settings->get_double("dry"));

  root.put(section + ".reverb.predelay", settings->get_double("predelay"));

  root.put(section + ".reverb.bass-cut", settings->get_double("bass-cut"));

  root.put(section + ".reverb.treble-cut", settings->get_double("treble-cut"));
}

void ReverbPreset::load(const boost::property_tree::ptree& root,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(root, settings, "input-gain", section + ".reverb.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".reverb.output-gain");

  update_string_key(root, settings, "room-size", section + ".reverb.room-size");

  update_key<double>(root, settings, "decay-time", section + ".reverb.decay-time");

  update_key<double>(root, settings, "hf-damp", section + ".reverb.hf-damp");

  update_key<double>(root, settings, "diffusion", section + ".reverb.diffusion");

  update_key<double>(root, settings, "amount", section + ".reverb.amount");

  update_key<double>(root, settings, "dry", section + ".reverb.dry");

  update_key<double>(root, settings, "predelay", section + ".reverb.predelay");

  update_key<double>(root, settings, "bass-cut", section + ".reverb.bass-cut");

  update_key<double>(root, settings, "treble-cut", section + ".reverb.treble-cut");
}

void ReverbPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void ReverbPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      load(root, "output", output_settings);
      break;
    case PresetType::input:
      load(root, "input", input_settings);
      break;
  }
}
