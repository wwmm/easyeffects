/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "compressor_preset.hpp"

CompressorPreset::CompressorPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::compressor::id,
                       tags::schema::compressor::input_path,
                       tags::schema::compressor::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::compressor).append("#").append(util::to_string(index));
}

void CompressorPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["dry"] = g_settings_get_double(settings, "dry");

  json[section][instance_name]["wet"] = g_settings_get_double(settings, "wet");

  json[section][instance_name]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section][instance_name]["attack"] = g_settings_get_double(settings, "attack");

  json[section][instance_name]["release"] = g_settings_get_double(settings, "release");

  json[section][instance_name]["release-threshold"] = g_settings_get_double(settings, "release-threshold");

  json[section][instance_name]["threshold"] = g_settings_get_double(settings, "threshold");

  json[section][instance_name]["ratio"] = g_settings_get_double(settings, "ratio");

  json[section][instance_name]["knee"] = g_settings_get_double(settings, "knee");

  json[section][instance_name]["makeup"] = g_settings_get_double(settings, "makeup");

  json[section][instance_name]["boost-threshold"] = g_settings_get_double(settings, "boost-threshold");

  json[section][instance_name]["boost-amount"] = g_settings_get_double(settings, "boost-amount");

  json[section][instance_name]["sidechain"]["type"] = util::gsettings_get_string(settings, "sidechain-type");

  json[section][instance_name]["sidechain"]["mode"] = util::gsettings_get_string(settings, "sidechain-mode");

  json[section][instance_name]["sidechain"]["source"] = util::gsettings_get_string(settings, "sidechain-source");

  json[section][instance_name]["sidechain"]["preamp"] = g_settings_get_double(settings, "sidechain-preamp");

  json[section][instance_name]["sidechain"]["reactivity"] = g_settings_get_double(settings, "sidechain-reactivity");

  json[section][instance_name]["sidechain"]["lookahead"] = g_settings_get_double(settings, "sidechain-lookahead");

  json[section][instance_name]["hpf-mode"] = util::gsettings_get_string(settings, "hpf-mode");

  json[section][instance_name]["hpf-frequency"] = g_settings_get_double(settings, "hpf-frequency");

  json[section][instance_name]["lpf-mode"] = util::gsettings_get_string(settings, "lpf-mode");

  json[section][instance_name]["lpf-frequency"] = g_settings_get_double(settings, "lpf-frequency");
}

void CompressorPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "dry", "dry");

  update_key<double>(json.at(section).at(instance_name), settings, "wet", "wet");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "mode", "mode");

  update_key<double>(json.at(section).at(instance_name), settings, "attack", "attack");

  update_key<double>(json.at(section).at(instance_name), settings, "release", "release");

  update_key<double>(json.at(section).at(instance_name), settings, "release-threshold", "release-threshold");

  update_key<double>(json.at(section).at(instance_name), settings, "threshold", "threshold");

  update_key<double>(json.at(section).at(instance_name), settings, "ratio", "ratio");

  update_key<double>(json.at(section).at(instance_name), settings, "knee", "knee");

  update_key<double>(json.at(section).at(instance_name), settings, "makeup", "makeup");

  update_key<double>(json.at(section).at(instance_name), settings, "boost-threshold", "boost-threshold");

  update_key<double>(json.at(section).at(instance_name), settings, "boost-amount", "boost-amount");

  update_key<gchar*>(json.at(section).at(instance_name).at("sidechain"), settings, "sidechain-type", "type");

  update_key<gchar*>(json.at(section).at(instance_name).at("sidechain"), settings, "sidechain-mode", "mode");

  update_key<gchar*>(json.at(section).at(instance_name).at("sidechain"), settings, "sidechain-source", "source");

  update_key<double>(json.at(section).at(instance_name).at("sidechain"), settings, "sidechain-preamp", "preamp");

  update_key<double>(json.at(section).at(instance_name).at("sidechain"), settings, "sidechain-reactivity",
                     "reactivity");

  update_key<double>(json.at(section).at(instance_name).at("sidechain"), settings, "sidechain-lookahead", "lookahead");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "hpf-mode", "hpf-mode");

  update_key<double>(json.at(section).at(instance_name), settings, "hpf-frequency", "hpf-frequency");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "lpf-mode", "lpf-mode");

  update_key<double>(json.at(section).at(instance_name), settings, "lpf-frequency", "lpf-frequency");
}
