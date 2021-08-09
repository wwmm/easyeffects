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

#include "limiter_preset.hpp"

LimiterPreset::LimiterPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.limiter",
                                         "/com/github/wwmm/easyeffects/streaminputs/limiter/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.limiter",
                                          "/com/github/wwmm/easyeffects/streamoutputs/limiter/");
}

void LimiterPreset::save(nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["limiter"]["mode"] = settings->get_string("mode").c_str();

  json[section]["limiter"]["oversampling"] = settings->get_string("oversampling").c_str();

  json[section]["limiter"]["dithering"] = settings->get_string("dithering").c_str();

  json[section]["limiter"]["input-gain"] = settings->get_double("input-gain");

  json[section]["limiter"]["output-gain"] = settings->get_double("output-gain");

  json[section]["limiter"]["lookahead"] = settings->get_double("lookahead");

  json[section]["limiter"]["attack"] = settings->get_double("attack");

  json[section]["limiter"]["release"] = settings->get_double("release");

  json[section]["limiter"]["threshold"] = settings->get_double("threshold");

  json[section]["limiter"]["sidechain-preamp"] = settings->get_double("sidechain-preamp");

  json[section]["limiter"]["stereo-link"] = settings->get_double("stereo-link");

  json[section]["limiter"]["alr-attack"] = settings->get_double("alr-attack");

  json[section]["limiter"]["alr-release"] = settings->get_double("alr-release");

  json[section]["limiter"]["alr-knee"] = settings->get_double("alr-knee");

  json[section]["limiter"]["alr"] = settings->get_boolean("alr");

  json[section]["limiter"]["gain-boost"] = settings->get_boolean("gain-boost");
}

void LimiterPreset::load(const nlohmann::json& json,
                         const std::string& section,
                         const Glib::RefPtr<Gio::Settings>& settings) {
  update_string_key(json.at(section).at("limiter"), settings, "mode", "mode");

  update_string_key(json.at(section).at("limiter"), settings, "oversampling", "oversampling");

  update_string_key(json.at(section).at("limiter"), settings, "dithering", "dithering");

  update_key<double>(json.at(section).at("limiter"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("limiter"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("limiter"), settings, "lookahead", "lookahead");

  update_key<double>(json.at(section).at("limiter"), settings, "attack", "attack");

  update_key<double>(json.at(section).at("limiter"), settings, "release", "release");

  update_key<double>(json.at(section).at("limiter"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("limiter"), settings, "sidechain-preamp", "sidechain-preamp");

  update_key<double>(json.at(section).at("limiter"), settings, "stereo-link", "stereo-link");

  update_key<double>(json.at(section).at("limiter"), settings, "alr-attack", "alr-attack");

  update_key<double>(json.at(section).at("limiter"), settings, "alr-release", "alr-release");

  update_key<double>(json.at(section).at("limiter"), settings, "alr-knee", "alr-knee");

  update_key<bool>(json.at(section).at("limiter"), settings, "alr", "alr");

  update_key<bool>(json.at(section).at("limiter"), settings, "gain-boost", "gain-boost");
}
