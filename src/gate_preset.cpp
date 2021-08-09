/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#include "gate_preset.hpp"

GatePreset::GatePreset() {
  input_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.gate", "/com/github/wwmm/easyeffects/streaminputs/gate/");

  output_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.gate", "/com/github/wwmm/easyeffects/streamoutputs/gate/");
}

void GatePreset::save(nlohmann::json& json, const std::string& section, const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["gate"]["input-gain"] = settings->get_double("input-gain");

  json[section]["gate"]["output-gain"] = settings->get_double("output-gain");

  json[section]["gate"]["detection"] = settings->get_string("detection").c_str();

  json[section]["gate"]["stereo-link"] = settings->get_string("stereo-link").c_str();

  json[section]["gate"]["range"] = settings->get_double("range");

  json[section]["gate"]["attack"] = settings->get_double("attack");

  json[section]["gate"]["release"] = settings->get_double("release");

  json[section]["gate"]["threshold"] = settings->get_double("threshold");

  json[section]["gate"]["ratio"] = settings->get_double("ratio");

  json[section]["gate"]["knee"] = settings->get_double("knee");

  json[section]["gate"]["makeup"] = settings->get_double("makeup");
}

void GatePreset::load(const nlohmann::json& json,
                      const std::string& section,
                      const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("gate"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("gate"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("gate"), settings, "detection", "detection");

  update_string_key(json.at(section).at("gate"), settings, "stereo-link", "stereo-link");

  update_key<double>(json.at(section).at("gate"), settings, "range", "range");

  update_key<double>(json.at(section).at("gate"), settings, "attack", "attack");

  update_key<double>(json.at(section).at("gate"), settings, "release", "release");

  update_key<double>(json.at(section).at("gate"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("gate"), settings, "ratio", "ratio");

  update_key<double>(json.at(section).at("gate"), settings, "knee", "knee");

  update_key<double>(json.at(section).at("gate"), settings, "makeup", "makeup");
}
