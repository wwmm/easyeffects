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

#include "bass_enhancer_preset.hpp"

BassEnhancerPreset::BassEnhancerPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::bass_enhancer::id,
                       tags::schema::bass_enhancer::input_path,
                       tags::schema::bass_enhancer::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::bass_enhancer).append("#").append(util::to_string(index));
}

void BassEnhancerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["amount"] = g_settings_get_double(settings, "amount");

  json[section][instance_name]["harmonics"] = g_settings_get_double(settings, "harmonics");

  json[section][instance_name]["scope"] = g_settings_get_double(settings, "scope");

  json[section][instance_name]["floor"] = g_settings_get_double(settings, "floor");

  json[section][instance_name]["blend"] = g_settings_get_double(settings, "blend");

  json[section][instance_name]["floor-active"] = g_settings_get_boolean(settings, "floor-active") != 0;
}

void BassEnhancerPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "amount", "amount");

  update_key<double>(json.at(section).at(instance_name), settings, "harmonics", "harmonics");

  update_key<double>(json.at(section).at(instance_name), settings, "scope", "scope");

  update_key<double>(json.at(section).at(instance_name), settings, "floor", "floor");

  update_key<double>(json.at(section).at(instance_name), settings, "blend", "blend");

  update_key<bool>(json.at(section).at(instance_name), settings, "floor-active", "floor-active");
}
