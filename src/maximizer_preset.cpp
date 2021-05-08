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

#include "maximizer_preset.hpp"

MaximizerPreset::MaximizerPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.easyeffects.maximizer",
                                            "/com/github/wwmm/easyeffects/streamoutputs/maximizer/")),
      input_settings(Gio::Settings::create("com.github.wwmm.easyeffects.maximizer",
                                           "/com/github/wwmm/easyeffects/streaminputs/maximizer/")) {}

void MaximizerPreset::save(boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".maximizer.release", settings->get_double("release"));

  root.put(section + ".maximizer.ceiling", settings->get_double("ceiling"));

  root.put(section + ".maximizer.threshold", settings->get_double("threshold"));
}

void MaximizerPreset::load(const boost::property_tree::ptree& root,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(root, settings, "release", section + ".maximizer.release");

  update_key<double>(root, settings, "ceiling", section + ".maximizer.ceiling");

  update_key<double>(root, settings, "threshold", section + ".maximizer.threshold");
}

void MaximizerPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      save(root, "output", output_settings);
      break;
    case PresetType::input:
      save(root, "input", input_settings);
      break;
  }
}

void MaximizerPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  switch (preset_type) {
    case PresetType::output:
      load(root, "output", output_settings);
      break;
    case PresetType::input:
      load(root, "input", input_settings);
      break;
  }
}
