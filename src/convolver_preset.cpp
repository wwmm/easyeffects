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

#include "convolver_preset.hpp"

ConvolverPreset::ConvolverPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::convolver::id,
                       tags::schema::convolver::input_path,
                       tags::schema::convolver::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::convolver).append("#").append(util::to_string(index));
}

void ConvolverPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["kernel-path"] = util::gsettings_get_string(settings, "kernel-path");

  json[section][instance_name]["ir-width"] = g_settings_get_int(settings, "ir-width");

  json[section][instance_name]["autogain"] = g_settings_get_boolean(settings, "autogain") != 0;
}

void ConvolverPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "kernel-path", "kernel-path");

  update_key<int>(json.at(section).at(instance_name), settings, "ir-width", "ir-width");

  update_key<bool>(json.at(section).at(instance_name), settings, "autogain", "autogain");
}
