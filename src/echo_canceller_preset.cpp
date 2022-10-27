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

#include "echo_canceller_preset.hpp"

EchoCancellerPreset::EchoCancellerPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::echo_canceller::id,
                       tags::schema::echo_canceller::input_path,
                       tags::schema::echo_canceller::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::echo_canceller).append("#").append(util::to_string(index));
}

void EchoCancellerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["frame-size"] = g_settings_get_int(settings, "frame-size");

  json[section][instance_name]["filter-length"] = g_settings_get_int(settings, "filter-length");

  json[section][instance_name]["residual-echo-suppression"] = g_settings_get_int(settings, "residual-echo-suppression");
}

void EchoCancellerPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at(instance_name), settings, "frame-size", "frame-size");

  update_key<int>(json.at(section).at(instance_name), settings, "filter-length", "filter-length");

  update_key<int>(json.at(section).at(instance_name), settings, "residual-echo-suppression",
                  "residual-echo-suppression");
}
