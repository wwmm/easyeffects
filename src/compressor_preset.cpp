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

#include "compressor_preset.hpp"

CompressorPreset::CompressorPreset() {
  input_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streaminputs/compressor/").c_str());

  output_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streamoutputs/compressor/").c_str());
}

void CompressorPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["compressor"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["compressor"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["compressor"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["compressor"]["attack"] = g_settings_get_double(settings, "attack");

  json[section]["compressor"]["release"] = g_settings_get_double(settings, "release");

  json[section]["compressor"]["release-threshold"] = g_settings_get_double(settings, "release-threshold");

  json[section]["compressor"]["threshold"] = g_settings_get_double(settings, "threshold");

  json[section]["compressor"]["ratio"] = g_settings_get_double(settings, "ratio");

  json[section]["compressor"]["knee"] = g_settings_get_double(settings, "knee");

  json[section]["compressor"]["makeup"] = g_settings_get_double(settings, "makeup");

  json[section]["compressor"]["boost-threshold"] = g_settings_get_double(settings, "boost-threshold");

  json[section]["compressor"]["boost-amount"] = g_settings_get_double(settings, "boost-amount");

  json[section]["compressor"]["sidechain"]["type"] = util::gsettings_get_string(settings, "sidechain-type");

  json[section]["compressor"]["sidechain"]["mode"] = util::gsettings_get_string(settings, "sidechain-mode");

  json[section]["compressor"]["sidechain"]["source"] = util::gsettings_get_string(settings, "sidechain-source");

  json[section]["compressor"]["sidechain"]["preamp"] = g_settings_get_double(settings, "sidechain-preamp");

  json[section]["compressor"]["sidechain"]["reactivity"] = g_settings_get_double(settings, "sidechain-reactivity");

  json[section]["compressor"]["sidechain"]["lookahead"] = g_settings_get_double(settings, "sidechain-lookahead");

  json[section]["compressor"]["hpf-mode"] = util::gsettings_get_string(settings, "hpf-mode");

  json[section]["compressor"]["hpf-frequency"] = g_settings_get_double(settings, "hpf-frequency");

  json[section]["compressor"]["lpf-mode"] = util::gsettings_get_string(settings, "lpf-mode");

  json[section]["compressor"]["lpf-frequency"] = g_settings_get_double(settings, "lpf-frequency");
}

void CompressorPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("compressor"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("compressor"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("compressor"), settings, "mode", "mode");

  update_key<double>(json.at(section).at("compressor"), settings, "attack", "attack");

  update_key<double>(json.at(section).at("compressor"), settings, "release", "release");

  update_key<double>(json.at(section).at("compressor"), settings, "release-threshold", "release-threshold");

  update_key<double>(json.at(section).at("compressor"), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at("compressor"), settings, "ratio", "ratio");

  update_key<double>(json.at(section).at("compressor"), settings, "knee", "knee");

  update_key<double>(json.at(section).at("compressor"), settings, "makeup", "makeup");

  update_key<double>(json.at(section).at("compressor"), settings, "boost-threshold", "boost-threshold");

  update_key<double>(json.at(section).at("compressor"), settings, "boost-amount", "boost-amount");

  update_key<gchar*>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-type", "type");

  update_key<gchar*>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-mode", "mode");

  update_key<gchar*>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-source", "source");

  update_key<double>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-preamp", "preamp");

  update_key<double>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-reactivity", "reactivity");

  update_key<double>(json.at(section).at("compressor").at("sidechain"), settings, "sidechain-lookahead", "lookahead");

  update_key<gchar*>(json.at(section).at("compressor"), settings, "hpf-mode", "hpf-mode");

  update_key<double>(json.at(section).at("compressor"), settings, "hpf-frequency", "hpf-frequency");

  update_key<gchar*>(json.at(section).at("compressor"), settings, "lpf-mode", "lpf-mode");

  update_key<double>(json.at(section).at("compressor"), settings, "lpf-frequency", "lpf-frequency");
}
