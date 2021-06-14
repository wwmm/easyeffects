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

#include "exciter_preset.hpp"

ExciterPreset::ExciterPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.exciter",
                                         "/com/github/wwmm/easyeffects/streaminputs/exciter/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.exciter",
                                          "/com/github/wwmm/easyeffects/streamoutputs/exciter/");
}

// void ExciterPreset::save(boost::property_tree::ptree& root,
//                          const std::string& section,
//                          const Glib::RefPtr<Gio::Settings>& settings) {
//   root.put(section + ".exciter.input-gain", settings->get_double("input-gain"));

//   root.put(section + ".exciter.output-gain", settings->get_double("output-gain"));

//   root.put(section + ".exciter.amount", settings->get_double("amount"));

//   root.put(section + ".exciter.harmonics", settings->get_double("harmonics"));

//   root.put(section + ".exciter.scope", settings->get_double("scope"));

//   root.put(section + ".exciter.ceil", settings->get_double("ceil"));

//   root.put(section + ".exciter.blend", settings->get_double("blend"));

//   root.put(section + ".exciter.ceil-active", settings->get_boolean("ceil-active"));
// }

void ExciterPreset::save(nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {}

void ExciterPreset::load(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("exciter"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("exciter"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("exciter"), settings, "amount", "amount");

  update_key<double>(json.at(section).at("exciter"), settings, "harmonics", "harmonics");

  update_key<double>(json.at(section).at("exciter"), settings, "scope", "scope");

  update_key<double>(json.at(section).at("exciter"), settings, "ceil", "ceil");

  update_key<double>(json.at(section).at("exciter"), settings, "blend", "blend");

  update_key<bool>(json.at(section).at("exciter"), settings, "ceil-active", "ceil-active");
}
