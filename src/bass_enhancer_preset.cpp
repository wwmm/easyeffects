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

#include "bass_enhancer_preset.hpp"

BassEnhancerPreset::BassEnhancerPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.bassenhancer",
                                         "/com/github/wwmm/easyeffects/streaminputs/bassenhancer/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.bassenhancer",
                                          "/com/github/wwmm/easyeffects/streamoutputs/bassenhancer/");
}

void BassEnhancerPreset::save(nlohmann::json& json,
                              const std::string& section,
                              const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["bass_enhancer"]["input-gain"] = settings->get_double("input-gain");

  json[section]["bass_enhancer"]["output-gain"] = settings->get_double("output-gain");

  json[section]["bass_enhancer"]["amount"] = settings->get_double("amount");

  json[section]["bass_enhancer"]["harmonics"] = settings->get_double("harmonics");

  json[section]["bass_enhancer"]["scope"] = settings->get_double("scope");

  json[section]["bass_enhancer"]["floor"] = settings->get_double("floor");

  json[section]["bass_enhancer"]["blend"] = settings->get_double("blend");

  json[section]["bass_enhancer"]["floor-active"] = settings->get_boolean("floor-active");
}

void BassEnhancerPreset::load(const nlohmann::json& json,
                              const std::string& section,
                              const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("bass_enhancer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "amount", "amount");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "harmonics", "harmonics");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "scope", "scope");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "floor", "floor");

  update_key<double>(json.at(section).at("bass_enhancer"), settings, "blend", "blend");

  update_key<bool>(json.at(section).at("bass_enhancer"), settings, "floor-active", "floor-active");
}
