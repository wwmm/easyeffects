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

#include "deesser_preset.hpp"

DeesserPreset::DeesserPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.deesser",
                                         "/com/github/wwmm/easyeffects/streaminputs/deesser/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.deesser",
                                          "/com/github/wwmm/easyeffects/streamoutputs/deesser/");
}

// void DeesserPreset::save(boost::property_tree::ptree& root,
//                          const std::string& section,
//                          const Glib::RefPtr<Gio::Settings>& settings) {
//   root.put(section + ".deesser.detection", settings->get_string("detection"));

//   root.put(section + ".deesser.mode", settings->get_string("mode"));

//   root.put(section + ".deesser.threshold", settings->get_double("threshold"));

//   root.put(section + ".deesser.ratio", settings->get_double("ratio"));

//   root.put(section + ".deesser.laxity", settings->get_int("laxity"));

//   root.put(section + ".deesser.makeup", settings->get_double("makeup"));

//   root.put(section + ".deesser.f1-freq", settings->get_double("f1-freq"));

//   root.put(section + ".deesser.f2-freq", settings->get_double("f2-freq"));

//   root.put(section + ".deesser.f1-level", settings->get_double("f1-level"));

//   root.put(section + ".deesser.f2-level", settings->get_double("f2-level"));

//   root.put(section + ".deesser.f2-q", settings->get_double("f2-q"));

//   root.put(section + ".deesser.sc-listen", settings->get_boolean("sc-listen"));
// }

void DeesserPreset::save(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {}

void DeesserPreset::load(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("deesser"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("deesser"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("deesser"), settings, "detection", "detection");

  update_string_key(json.at(section).at("deesser"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("deesser"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("deesser"), settings, "ratio", "ratio");

  update_key<int>(json.at(section).at("deesser"), settings, "laxity", "laxity");

  update_key<double>(json.at(section).at("deesser"), settings, "makeup", "makeup");

  update_key<double>(json.at(section).at("deesser"), settings, "f1-freq", "f1-freq");

  update_key<double>(json.at(section).at("deesser"), settings, "f2-freq", "f2-freq");

  update_key<double>(json.at(section).at("deesser"), settings, "f1-level", "f1-level");

  update_key<double>(json.at(section).at("deesser"), settings, "f2-level", "f2-level");

  update_key<double>(json.at(section).at("deesser"), settings, "f2-q", "f2-q");

  update_key<bool>(json.at(section).at("deesser"), settings, "sc-listen", "sc-listen");
}
