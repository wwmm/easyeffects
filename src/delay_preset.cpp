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

#include "delay_preset.hpp"

DelayPreset::DelayPreset()
    : output_settings(Gio::Settings::create("com.github.wwmm.pulseeffects.delay",
                                            "/com/github/wwmm/pulseeffects/sinkinputs/delay/")) {}

void DelayPreset::save(boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  root.put(section + ".delay.input-gain", settings->get_double("input-gain"));

  root.put(section + ".delay.output-gain", settings->get_double("output-gain"));

  root.put(section + ".delay.time-l", settings->get_double("time-l"));
  root.put(section + ".delay.time-r", settings->get_double("time-r"));
}

void DelayPreset::load(const boost::property_tree::ptree& root,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(root, settings, "input-gain", section + ".delay.input-gain");

  update_key<double>(root, settings, "output-gain", section + ".delay.output-gain");

  update_key<double>(root, settings, "time-l", section + ".delay.time-l");

  update_key<double>(root, settings, "time-r", section + ".delay.time-r");
}

void DelayPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    save(root, "output", output_settings);
  }
}

void DelayPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {
  if (preset_type == PresetType::output) {
    load(root, "output", output_settings);
  }
}
