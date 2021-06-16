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

#include "reverb_preset.hpp"

ReverbPreset::ReverbPreset() {
  input_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.reverb", "/com/github/wwmm/easyeffects/streaminputs/reverb/");

  output_settings =
      Gio::Settings::create("com.github.wwmm.easyeffects.reverb", "/com/github/wwmm/easyeffects/streamoutputs/reverb/");
}

void ReverbPreset::save(nlohmann::json& json, const std::string& section, const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["reverb"]["input-gain"] = settings->get_double("input-gain");

  json[section]["reverb"]["output-gain"] = settings->get_double("output-gain");

  json[section]["reverb"]["room-size"] = settings->get_string("room-size").c_str();

  json[section]["reverb"]["decay-time"] = settings->get_double("decay-time");

  json[section]["reverb"]["hf-damp"] = settings->get_double("hf-damp");

  json[section]["reverb"]["diffusion"] = settings->get_double("diffusion");

  json[section]["reverb"]["amount"] = settings->get_double("amount");

  json[section]["reverb"]["dry"] = settings->get_double("dry");

  json[section]["reverb"]["predelay"] = settings->get_double("predelay");

  json[section]["reverb"]["bass-cut"] = settings->get_double("bass-cut");

  json[section]["reverb"]["treble-cut"] = settings->get_double("treble-cut");
}

void ReverbPreset::load(const nlohmann::json& json,
                        const std::string& section,
                        const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("reverb"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("reverb"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("reverb"), settings, "room-size", "room-size");

  update_key<double>(json.at(section).at("reverb"), settings, "decay-time", "decay-time");

  update_key<double>(json.at(section).at("reverb"), settings, "hf-damp", "hf-damp");

  update_key<double>(json.at(section).at("reverb"), settings, "diffusion", "diffusion");

  update_key<double>(json.at(section).at("reverb"), settings, "amount", "amount");

  update_key<double>(json.at(section).at("reverb"), settings, "dry", "dry");

  update_key<double>(json.at(section).at("reverb"), settings, "predelay", "predelay");

  update_key<double>(json.at(section).at("reverb"), settings, "bass-cut", "bass-cut");

  update_key<double>(json.at(section).at("reverb"), settings, "treble-cut", "treble-cut");
}
