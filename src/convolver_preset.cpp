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

#include "convolver_preset.hpp"

ConvolverPreset::ConvolverPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.convolver",
                                         "/com/github/wwmm/easyeffects/streaminputs/convolver/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.convolver",
                                          "/com/github/wwmm/easyeffects/streamoutputs/convolver/");
}

void ConvolverPreset::save(nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["convolver"]["input-gain"] = settings->get_double("input-gain");

  json[section]["convolver"]["output-gain"] = settings->get_double("output-gain");

  json[section]["convolver"]["kernel-path"] = settings->get_string("kernel-path").c_str();

  json[section]["convolver"]["ir-width"] = settings->get_int("ir-width");
}

void ConvolverPreset::load(const nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("convolver"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("convolver"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("convolver"), settings, "kernel-path", "kernel-path");

  update_key<int>(json.at(section).at("convolver"), settings, "ir-width", "ir-width");
}
