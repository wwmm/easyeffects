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

#include "speex_preset.hpp"

SpeexPreset::SpeexPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::speex::id,
                       tags::schema::speex::input_path,
                       tags::schema::speex::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::speex).append("#").append(util::to_string(index));
}

void SpeexPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["enable-denoise"] = g_settings_get_boolean(settings, "enable-denoise") != 0;

  json[section][instance_name]["noise-suppression"] = g_settings_get_int(settings, "noise-suppression");

  json[section][instance_name]["enable-agc"] = g_settings_get_boolean(settings, "enable-agc") != 0;

  json[section][instance_name]["vad"]["enable"] = g_settings_get_boolean(settings, "enable-vad") != 0;

  json[section][instance_name]["vad"]["probability-start"] = g_settings_get_int(settings, "vad-probability-start");

  json[section][instance_name]["vad"]["probability-continue"] =
      g_settings_get_int(settings, "vad-probability-continue");

  json[section][instance_name]["enable-dereverb"] = g_settings_get_boolean(settings, "enable-dereverb") != 0;
}

void SpeexPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<bool>(json.at(section).at(instance_name), settings, "enable-denoise", "enable-denoise");

  update_key<int>(json.at(section).at(instance_name), settings, "noise-suppression", "noise-suppression");

  update_key<bool>(json.at(section).at(instance_name), settings, "enable-agc", "enable-agc");

  update_key<bool>(json.at(section).at(instance_name).at("vad"), settings, "enable-vad", "enable");

  update_key<int>(json.at(section).at(instance_name).at("vad"), settings, "vad-probability-start", "probability-start");

  update_key<int>(json.at(section).at(instance_name).at("vad"), settings, "vad-probability-continue",
                  "probability-continue");

  update_key<bool>(json.at(section).at(instance_name), settings, "enable-dereverb", "enable-dereverb");
}
