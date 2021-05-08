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

#include "loudness_preset.hpp"

LoudnessPreset::LoudnessPreset()
    : input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.loudness",
                                           "/com/github/wwmm/easyeffects/streaminputs/loudness/")),
      output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.loudness",
                                            "/com/github/wwmm/easyeffects/streamoutputs/loudness/")) {}

void LoudnessPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".loudness.fft", settings->get_string("fft"));

  root.put(section + ".loudness.std", settings->get_string("std"));

  root.put(section + ".loudness.volume", settings->get_double("volume"));
}

void LoudnessPreset::load(const boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_string_key(root, settings, "fft", section + ".loudness.fft");

  update_string_key(root, settings, "std", section + ".loudness.std");

  update_key<double>(root, settings, "volume", section + ".loudness.volume");
}

void LoudnessPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void LoudnessPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      load(root, "output", output_settings);
      break;
    case PresetType::input:
      load(root, "input", input_settings);
      break;
  }
}
