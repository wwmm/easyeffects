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

#include "spectrum_preset.hpp"

SpectrumPreset::SpectrumPreset() : settings(Gio::Settings::create("com.github.wwmm.easyeffects.spectrum")) {}

void SpectrumPreset::save(boost::property_tree::ptree& root,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  boost::property_tree::ptree node_in;
  Glib::Variant<std::vector<double>> aux;

  root.put("spectrum.show", settings->get_boolean("show"));

  root.put("spectrum.n-points", settings->get_int("n-points"));

  root.put("spectrum.height", settings->get_int("height"));

  root.put("spectrum.fill", settings->get_boolean("fill"));

  root.put("spectrum.show-bar-border", settings->get_boolean("show-bar-border"));

  root.put("spectrum.line-width", settings->get_double("line-width"));

  root.put("spectrum.type", settings->get_string("type"));

  // color

  settings->get_value("color", aux);

  for (const auto& p : aux.get()) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("spectrum.color", node_in);

  // axis color

  node_in.clear();

  settings->get_value("color-axis-labels", aux);

  for (const auto& p : aux.get()) {
    boost::property_tree::ptree node;
    node.put("", p);
    node_in.push_back(std::make_pair("", node));
  }

  root.add_child("spectrum.color-axis-labels", node_in);
}

void SpectrumPreset::load(const nlohmann::json& json,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<bool>(json.at("spectrum"), settings, "show", "show");

  update_key<int>(json.at("spectrum"), settings, "n-points", "n-points");

  update_key<int>(json.at("spectrum"), settings, "height", "height");

  update_key<bool>(json.at("spectrum"), settings, "fill", "fill");

  update_key<bool>(json.at("spectrum"), settings, "show-bar-border", "show-bar-border");

  update_key<double>(json.at("spectrum"), settings, "line-width", "line-width");

  update_string_key(json.at("spectrum"), settings, "type", "type");

  // spectrum color

  auto color = json.at("spectrum").at("color").get<std::vector<double>>();

  settings->set_value("color", Glib::Variant<std::vector<double>>::create(color));

  // axis color

  color = json.at("spectrum").at("color-axis-labels").get<std::vector<double>>();

  settings->set_value("color-axis-labels", Glib::Variant<std::vector<double>>::create(color));
}

void SpectrumPreset::write(PresetType preset_type, boost::property_tree::ptree& root) {
  save(root, "", settings);
}

void SpectrumPreset::read(PresetType preset_type, const boost::property_tree::ptree& root) {}

void SpectrumPreset::read(PresetType preset_type, const nlohmann::json& json) {
  try {
    load(json, "", settings);
  } catch (const nlohmann::json::exception& e) {
    util::warning(e.what());
  }
}
