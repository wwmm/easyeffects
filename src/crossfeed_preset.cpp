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

#include "crossfeed_preset.hpp"

CrossfeedPreset::CrossfeedPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.crossfeed",
                                         "/com/github/wwmm/easyeffects/streaminputs/crossfeed/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.crossfeed",
                                          "/com/github/wwmm/easyeffects/streamoutputs/crossfeed/");
}

void CrossfeedPreset::save(nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["crossfeed"]["input-gain"] = settings->get_double("input-gain");

  json[section]["crossfeed"]["output-gain"] = settings->get_double("output-gain");

  json[section]["crossfeed"]["fcut"] = settings->get_int("fcut");

  json[section]["crossfeed"]["feed"] = settings->get_double("feed");
}

void CrossfeedPreset::load(const nlohmann::json& json,
                           const std::string& section,
                           const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("crossfeed"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("crossfeed"), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at("crossfeed"), settings, "fcut", "fcut");

  update_key<double>(json.at(section).at("crossfeed"), settings, "feed", "feed");
}
