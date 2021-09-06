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

#include "maximizer_preset.hpp"

MaximizerPreset::MaximizerPreset() {
  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.maximizer",
                                          "/com/github/wwmm/easyeffects/streamoutputs/maximizer/");

  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.maximizer",
                                         "/com/github/wwmm/easyeffects/streaminputs/maximizer/");
}

void MaximizerPreset::save(nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["maximizer"]["input-gain"] = settings->get_double("input-gain");

  json[section]["maximizer"]["output-gain"] = settings->get_double("output-gain");

  json[section]["maximizer"]["release"] = settings->get_double("release");

  json[section]["maximizer"]["ceiling"] = settings->get_double("ceiling");

  json[section]["maximizer"]["threshold"] = settings->get_double("threshold");
}

void MaximizerPreset::load(const nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("maximizer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("maximizer"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("maximizer"), settings, "release", "release");

  update_key<double>(json.at(section).at("maximizer"), settings, "ceiling", "ceiling");

  update_key<double>(json.at(section).at("maximizer"), settings, "threshold", "threshold");
}
