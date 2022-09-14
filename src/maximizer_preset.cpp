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

#include "maximizer_preset.hpp"

MaximizerPreset::MaximizerPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::maximizer::id,
                       tags::schema::maximizer::input_path,
                       tags::schema::maximizer::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::maximizer).append("#").append(util::to_string(index));
}

void MaximizerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["release"] = g_settings_get_double(settings, "release");

  json[section][instance_name]["ceiling"] = g_settings_get_double(settings, "ceiling");

  json[section][instance_name]["threshold"] = g_settings_get_double(settings, "threshold");
}

void MaximizerPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "release", "release");

  update_key<double>(json.at(section).at(instance_name), settings, "ceiling", "ceiling");

  update_key<double>(json.at(section).at(instance_name), settings, "threshold", "threshold");
}
