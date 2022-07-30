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

#include "gate_preset.hpp"

GatePreset::GatePreset(PresetType preset_type, const int& index) : PluginPresetBase(preset_type, index) {
  switch (preset_type) {
    case PresetType::input:
      settings = g_settings_new_with_path(tags::schema::gate::id, tags::schema::gate::input_path);
      break;
    case PresetType::output:
      settings = g_settings_new_with_path(tags::schema::gate::id, tags::schema::gate::output_path);
      break;
  }
}

void GatePreset::save(nlohmann::json& json) {
  json[section]["gate"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["gate"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["gate"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["gate"]["dry"] = g_settings_get_double(settings, "dry");

  json[section]["gate"]["wet"] = g_settings_get_double(settings, "wet");

  json[section]["gate"]["attack"] = g_settings_get_double(settings, "attack");

  json[section]["gate"]["release"] = g_settings_get_double(settings, "release");

  json[section]["gate"]["curve-threshold"] = g_settings_get_double(settings, "curve-threshold");

  json[section]["gate"]["curve-zone"] = g_settings_get_double(settings, "curve-zone");

  json[section]["gate"]["hysteresis"] = g_settings_get_boolean(settings, "hysteresis") != 0;

  json[section]["gate"]["hysteresis-threshold"] = g_settings_get_double(settings, "hysteresis-threshold");

  json[section]["gate"]["hysteresis-zone"] = g_settings_get_double(settings, "hysteresis-zone");

  json[section]["gate"]["reduction"] = g_settings_get_double(settings, "reduction");

  json[section]["gate"]["makeup"] = g_settings_get_double(settings, "makeup");

  json[section]["gate"]["sidechain"]["input"] = util::gsettings_get_string(settings, "sidechain-input");

  json[section]["gate"]["sidechain"]["mode"] = util::gsettings_get_string(settings, "sidechain-mode");

  json[section]["gate"]["sidechain"]["source"] = util::gsettings_get_string(settings, "sidechain-source");

  json[section]["gate"]["sidechain"]["preamp"] = g_settings_get_double(settings, "sidechain-preamp");

  json[section]["gate"]["sidechain"]["reactivity"] = g_settings_get_double(settings, "sidechain-reactivity");

  json[section]["gate"]["sidechain"]["lookahead"] = g_settings_get_double(settings, "sidechain-lookahead");

  json[section]["gate"]["hpf-mode"] = util::gsettings_get_string(settings, "hpf-mode");

  json[section]["gate"]["hpf-frequency"] = g_settings_get_double(settings, "hpf-frequency");

  json[section]["gate"]["lpf-mode"] = util::gsettings_get_string(settings, "lpf-mode");

  json[section]["gate"]["lpf-frequency"] = g_settings_get_double(settings, "lpf-frequency");
}

void GatePreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at("gate"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("gate"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("gate"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("gate"), settings, "dry", "dry");

  update_key<double>(json.at(section).at("gate"), settings, "wet", "wet");

  update_key<double>(json.at(section).at("gate"), settings, "attack", "attack");

  update_key<double>(json.at(section).at("gate"), settings, "release", "release");

  update_key<double>(json.at(section).at("gate"), settings, "curve-threshold", "curve-threshold");

  update_key<double>(json.at(section).at("gate"), settings, "curve-zone", "curve-zone");

  update_key<bool>(json.at(section).at("gate"), settings, "hysteresis", "hysteresis");

  update_key<double>(json.at(section).at("gate"), settings, "hysteresis-threshold", "hysteresis-threshold");

  update_key<double>(json.at(section).at("gate"), settings, "hysteresis-zone", "hysteresis-zone");

  update_key<double>(json.at(section).at("gate"), settings, "reduction", "reduction");

  update_key<double>(json.at(section).at("gate"), settings, "makeup", "makeup");

  update_key<gchar*>(json.at(section).at("gate").at("sidechain"), settings, "sidechain-input", "input");

  update_key<gchar*>(json.at(section).at("gate").at("sidechain"), settings, "sidechain-mode", "mode");

  update_key<gchar*>(json.at(section).at("gate").at("sidechain"), settings, "sidechain-source", "source");

  update_key<double>(json.at(section).at("gate").at("sidechain"), settings, "sidechain-preamp", "preamp");

  update_key<double>(json.at(section).at("gate").at("sidechain"), settings, "sidechain-reactivity", "reactivity");

  update_key<double>(json.at(section).at("gate").at("sidechain"), settings, "sidechain-lookahead", "lookahead");

  update_key<gchar*>(json.at(section).at("gate"), settings, "hpf-mode", "hpf-mode");

  update_key<double>(json.at(section).at("gate"), settings, "hpf-frequency", "hpf-frequency");

  update_key<gchar*>(json.at(section).at("gate"), settings, "lpf-mode", "lpf-mode");

  update_key<double>(json.at(section).at("gate"), settings, "lpf-frequency", "lpf-frequency");
}
