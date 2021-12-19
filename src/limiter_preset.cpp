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
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.limiter",
                                            "/com/github/wwmm/easyeffects/streaminputs/limiter/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.limiter",
                                             "/com/github/wwmm/easyeffects/streamoutputs/limiter/");
}

void LimiterPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["limiter"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["limiter"]["oversampling"] = util::gsettings_get_string(settings, "oversampling");

  json[section]["limiter"]["dithering"] = util::gsettings_get_string(settings, "dithering");

  json[section]["limiter"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["limiter"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["limiter"]["lookahead"] = g_settings_get_double(settings, "lookahead");

  json[section]["limiter"]["attack"] = g_settings_get_double(settings, "attack");

  json[section]["limiter"]["release"] = g_settings_get_double(settings, "release");

  json[section]["limiter"]["threshold"] = g_settings_get_double(settings, "threshold");

  json[section]["limiter"]["sidechain-preamp"] = g_settings_get_double(settings, "sidechain-preamp");

  json[section]["limiter"]["stereo-link"] = g_settings_get_double(settings, "stereo-link");

  json[section]["limiter"]["alr-attack"] = g_settings_get_double(settings, "alr-attack");

  json[section]["limiter"]["alr-release"] = g_settings_get_double(settings, "alr-release");

  json[section]["limiter"]["alr-knee"] = g_settings_get_double(settings, "alr-knee");

  json[section]["limiter"]["alr"] = g_settings_get_boolean(settings, "alr") != 0;

  json[section]["limiter"]["gain-boost"] = g_settings_get_boolean(settings, "gain-boost") != 0;

  json[section]["limiter"]["external-sidechain"] = g_settings_get_boolean(settings, "external-sidechain") != 0;
}

void LimiterPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<gchar*>(json.at(section).at("limiter"), settings, "mode", "mode");

  update_key<gchar*>(json.at(section).at("limiter"), settings, "oversampling", "oversampling");

  update_key<gchar*>(json.at(section).at("limiter"), settings, "dithering", "dithering");

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

  update_key<bool>(json.at(section).at("limiter"), settings, "external-sidechain", "external-sidechain");
}
