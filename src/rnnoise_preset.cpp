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

#include "rnnoise_preset.hpp"

RNNoisePreset::RNNoisePreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.rnnoise",
                                         "/com/github/wwmm/easyeffects/streaminputs/rnnoise/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.rnnoise",
                                          "/com/github/wwmm/easyeffects/streamoutputs/rnnoise/");
}

void RNNoisePreset::save(nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["rnnoise"]["input-gain"] = settings->get_double("input-gain");

  json[section]["rnnoise"]["output-gain"] = settings->get_double("output-gain");

  json[section]["rnnoise"]["model-path"] = settings->get_string("model-path").c_str();
}

void RNNoisePreset::load(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("rnnoise"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("rnnoise"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("rnnoise"), settings, "model-path", "model-path");
}
