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

#include "bass_loudness_preset.hpp"

BassLoudnessPreset::BassLoudnessPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::bass_loudness::id,
                       tags::schema::bass_loudness::input_path,
                       tags::schema::bass_loudness::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::bass_loudness).append("#").append(util::to_string(index));
}

void BassLoudnessPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["loudness"] = g_settings_get_double(settings, "loudness");

  json[section][instance_name]["output"] = g_settings_get_double(settings, "output");

  json[section][instance_name]["link"] = g_settings_get_double(settings, "link");
}

void BassLoudnessPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "loudness", "loudness");

  update_key<double>(json.at(section).at(instance_name), settings, "output", "output");

  update_key<double>(json.at(section).at(instance_name), settings, "link", "link");
}
