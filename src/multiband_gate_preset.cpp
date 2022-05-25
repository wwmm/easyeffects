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

#include "multiband_gate_preset.hpp"

MultibandGatePreset::MultibandGatePreset() {
  input_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streaminputs/multibandgate/").c_str());

  output_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streamoutputs/multibandgate/").c_str());
}

void MultibandGatePreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["multiband_gate"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["multiband_gate"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["multiband_gate"]["freq0"] = g_settings_get_double(settings, "freq0");

  json[section]["multiband_gate"]["freq1"] = g_settings_get_double(settings, "freq1");

  json[section]["multiband_gate"]["freq2"] = g_settings_get_double(settings, "freq2");

  json[section]["multiband_gate"]["mode"] = util::gsettings_get_string(settings, "mode");

  // sub band

  json[section]["multiband_gate"]["subband"]["reduction"] = g_settings_get_double(settings, "range0");

  json[section]["multiband_gate"]["subband"]["threshold"] = g_settings_get_double(settings, "threshold0");

  json[section]["multiband_gate"]["subband"]["ratio"] = g_settings_get_double(settings, "ratio0");

  json[section]["multiband_gate"]["subband"]["attack"] = g_settings_get_double(settings, "attack0");

  json[section]["multiband_gate"]["subband"]["release"] = g_settings_get_double(settings, "release0");

  json[section]["multiband_gate"]["subband"]["makeup"] = g_settings_get_double(settings, "makeup0");

  json[section]["multiband_gate"]["subband"]["knee"] = g_settings_get_double(settings, "knee0");

  json[section]["multiband_gate"]["subband"]["detection"] = util::gsettings_get_string(settings, "detection0");

  json[section]["multiband_gate"]["subband"]["bypass"] = g_settings_get_boolean(settings, "bypass0") != 0;

  json[section]["multiband_gate"]["subband"]["solo"] = g_settings_get_boolean(settings, "solo0") != 0;

  // low band

  json[section]["multiband_gate"]["lowband"]["reduction"] = g_settings_get_double(settings, "range1");

  json[section]["multiband_gate"]["lowband"]["threshold"] = g_settings_get_double(settings, "threshold1");

  json[section]["multiband_gate"]["lowband"]["ratio"] = g_settings_get_double(settings, "ratio1");

  json[section]["multiband_gate"]["lowband"]["attack"] = g_settings_get_double(settings, "attack1");

  json[section]["multiband_gate"]["lowband"]["release"] = g_settings_get_double(settings, "release1");

  json[section]["multiband_gate"]["lowband"]["makeup"] = g_settings_get_double(settings, "makeup1");

  json[section]["multiband_gate"]["lowband"]["knee"] = g_settings_get_double(settings, "knee1");

  json[section]["multiband_gate"]["lowband"]["detection"] = util::gsettings_get_string(settings, "detection1");

  json[section]["multiband_gate"]["lowband"]["bypass"] = g_settings_get_boolean(settings, "bypass1") != 0;

  json[section]["multiband_gate"]["lowband"]["solo"] = g_settings_get_boolean(settings, "solo1") != 0;

  // mid band

  json[section]["multiband_gate"]["midband"]["reduction"] = g_settings_get_double(settings, "range2");

  json[section]["multiband_gate"]["midband"]["threshold"] = g_settings_get_double(settings, "threshold2");

  json[section]["multiband_gate"]["midband"]["ratio"] = g_settings_get_double(settings, "ratio2");

  json[section]["multiband_gate"]["midband"]["attack"] = g_settings_get_double(settings, "attack2");

  json[section]["multiband_gate"]["midband"]["release"] = g_settings_get_double(settings, "release2");

  json[section]["multiband_gate"]["midband"]["makeup"] = g_settings_get_double(settings, "makeup2");

  json[section]["multiband_gate"]["midband"]["knee"] = g_settings_get_double(settings, "knee2");

  json[section]["multiband_gate"]["midband"]["detection"] = util::gsettings_get_string(settings, "detection2");

  json[section]["multiband_gate"]["midband"]["bypass"] = g_settings_get_boolean(settings, "bypass2") != 0;

  json[section]["multiband_gate"]["midband"]["solo"] = g_settings_get_boolean(settings, "solo2") != 0;

  // high band

  json[section]["multiband_gate"]["highband"]["reduction"] = g_settings_get_double(settings, "range3");

  json[section]["multiband_gate"]["highband"]["threshold"] = g_settings_get_double(settings, "threshold3");

  json[section]["multiband_gate"]["highband"]["ratio"] = g_settings_get_double(settings, "ratio3");

  json[section]["multiband_gate"]["highband"]["attack"] = g_settings_get_double(settings, "attack3");

  json[section]["multiband_gate"]["highband"]["release"] = g_settings_get_double(settings, "release3");

  json[section]["multiband_gate"]["highband"]["makeup"] = g_settings_get_double(settings, "makeup3");

  json[section]["multiband_gate"]["highband"]["knee"] = g_settings_get_double(settings, "knee3");

  json[section]["multiband_gate"]["highband"]["detection"] = util::gsettings_get_string(settings, "detection3");

  json[section]["multiband_gate"]["highband"]["bypass"] = g_settings_get_boolean(settings, "bypass3") != 0;

  json[section]["multiband_gate"]["highband"]["solo"] = g_settings_get_boolean(settings, "solo3") != 0;
}

void MultibandGatePreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("multiband_gate"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "freq0", "freq0");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "freq1", "freq1");

  update_key<double>(json.at(section).at("multiband_gate"), settings, "freq2", "freq2");

  update_key<gchar*>(json.at(section).at("multiband_gate"), settings, "mode", "mode");

  // sub band

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "range0", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "threshold0", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "ratio0", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "attack0", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "release0", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "makeup0", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("subband"), settings, "knee0", "knee");

  update_key<gchar*>(json.at(section).at("multiband_gate").at("subband"), settings, "detection0", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("subband"), settings, "bypass0", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("subband"), settings, "solo0", "solo");

  // low band

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "range1", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "threshold1", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "ratio1", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "attack1", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "release1", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "makeup1", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("lowband"), settings, "knee1", "knee");

  update_key<gchar*>(json.at(section).at("multiband_gate").at("lowband"), settings, "detection1", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("lowband"), settings, "bypass1", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("lowband"), settings, "solo1", "solo");

  // mid band

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "range2", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "threshold2", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "ratio2", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "attack2", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "release2", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "makeup2", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("midband"), settings, "knee2", "knee");

  update_key<gchar*>(json.at(section).at("multiband_gate").at("midband"), settings, "detection2", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("midband"), settings, "bypass2", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("midband"), settings, "solo2", "solo");

  // high band

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "range3", "reduction");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "threshold3", "threshold");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "ratio3", "ratio");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "attack3", "attack");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "release3", "release");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "makeup3", "makeup");

  update_key<double>(json.at(section).at("multiband_gate").at("highband"), settings, "knee3", "knee");

  update_key<gchar*>(json.at(section).at("multiband_gate").at("highband"), settings, "detection3", "detection");

  update_key<bool>(json.at(section).at("multiband_gate").at("highband"), settings, "bypass3", "bypass");

  update_key<bool>(json.at(section).at("multiband_gate").at("highband"), settings, "solo3", "solo");
}
