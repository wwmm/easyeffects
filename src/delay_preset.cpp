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

#include "delay_preset.hpp"

DelayPreset::DelayPreset() {
  input_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.delay", "/com/github/wwmm/easyeffects/streaminputs/delay/");

  output_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.delay", "/com/github/wwmm/easyeffects/streamoutputs/delay/");
}

// void DelayPreset::save(boost::property_tree::ptree& root,
//                        const std::string& section,
//                        const Glib::RefPtr<Gio::Settings>& settings) {
//   root.put(section + ".delay.input-gain", settings->get_double("input-gain"));

//   root.put(section + ".delay.output-gain", settings->get_double("output-gain"));

//   root.put(section + ".delay.time-l", settings->get_double("time-l"));
//   root.put(section + ".delay.time-r", settings->get_double("time-r"));
// }

void DelayPreset::save(const nlohmann::json& json,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {}

void DelayPreset::load(const nlohmann::json& json,
                       const std::string& section,
                       const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("delay"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("delay"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("delay"), settings, "time-l", "time-l");

  update_key<double>(json.at(section).at("delay"), settings, "time-r", "time-r");
}
