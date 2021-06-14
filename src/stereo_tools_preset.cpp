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

#include "stereo_tools_preset.hpp"

StereoToolsPreset::StereoToolsPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.stereotools",
                                         "/com/github/wwmm/easyeffects/streaminputs/stereotools/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.stereotools",
                                          "/com/github/wwmm/easyeffects/streamoutputs/stereotools/");
}

// void StereoToolsPreset::save(boost::property_tree::ptree& root,
//                              const std::string& section,
//                              const Glib::RefPtr<Gio::Settings>& settings) {
//   root.put(section + ".stereo_tools.input-gain", settings->get_double("input-gain"));

//   root.put(section + ".stereo_tools.output-gain", settings->get_double("output-gain"));

//   root.put(section + ".stereo_tools.balance-in", settings->get_double("balance-in"));

//   root.put(section + ".stereo_tools.balance-out", settings->get_double("balance-out"));

//   root.put(section + ".stereo_tools.softclip", settings->get_boolean("softclip"));

//   root.put(section + ".stereo_tools.mutel", settings->get_boolean("mutel"));

//   root.put(section + ".stereo_tools.muter", settings->get_boolean("muter"));

//   root.put(section + ".stereo_tools.phasel", settings->get_boolean("phasel"));

//   root.put(section + ".stereo_tools.phaser", settings->get_boolean("phaser"));

//   root.put(section + ".stereo_tools.mode", settings->get_string("mode"));

//   root.put(section + ".stereo_tools.side-level", settings->get_double("slev"));

//   root.put(section + ".stereo_tools.side-balance", settings->get_double("sbal"));

//   root.put(section + ".stereo_tools.middle-level", settings->get_double("mlev"));

//   root.put(section + ".stereo_tools.middle-panorama", settings->get_double("mpan"));

//   root.put(section + ".stereo_tools.stereo-base", settings->get_double("stereo-base"));

//   root.put(section + ".stereo_tools.delay", settings->get_double("delay"));

//   root.put(section + ".stereo_tools.sc-level", settings->get_double("sc-level"));

//   root.put(section + ".stereo_tools.stereo-phase", settings->get_double("stereo-phase"));
// }

void StereoToolsPreset::save(nlohmann::json& json,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {}

void StereoToolsPreset::load(const nlohmann::json& json,
                             const std::string& section,
                             const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("stereo_tools"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "balance-in", "balance-in");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "balance-out", "balance-out");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "softclip", "softclip");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "mutel", "mutel");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "muter", "muter");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "phasel", "phasel");

  update_key<bool>(json.at(section).at("stereo_tools"), settings, "phaser", "phaser");

  update_string_key(json.at(section).at("stereo_tools"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "slev", "side-level");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "sbal", "side-balance");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "mlev", "middle-level");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "mpan", "middle-panorama");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "stereo-base", "stereo-base");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "delay", "delay");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "sc-level", "sc-level");

  update_key<double>(json.at(section).at("stereo_tools"), settings, "stereo-phase", "stereo-phase");
}
