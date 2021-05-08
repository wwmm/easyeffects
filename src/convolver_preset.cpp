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

#include "convolver_preset.hpp"

ConvolverPreset::ConvolverPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.convolver",
                                            "/com/github/wwmm/easyeffects/streamoutputs/convolver/")) {}

void ConvolverPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".convolver.input-gain", settings->get_double("input-gain"));

  root.put(section + ".convolver.output-gain", settings->get_double("output-gain"));

  root.put(section + ".convolver.kernel-path", settings->get_string("kernel-path"));

  root.put(section + ".convolver.ir-width", settings->get_int("ir-width"));
}

void ConvolverPreset::load(const boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(root, settings, "input-gain", section + ".convolver.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".convolver.output-gain");

  update_string_key(root, settings, "kernel-path", section + ".convolver.kernel-path");

  update_key<int>(root, settings, "ir-width", section + ".convolver.ir-width");
}

void ConvolverPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void ConvolverPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
