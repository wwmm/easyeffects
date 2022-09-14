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

#include "crystalizer_preset.hpp"

CrystalizerPreset::CrystalizerPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::crystalizer::id,
                       tags::schema::crystalizer::input_path,
                       tags::schema::crystalizer::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::crystalizer).append("#").append(util::to_string(index));
}

void CrystalizerPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  for (int n = 0; n < 13; n++) {
    const auto bandn = "band" + util::to_string(n);

    json[section][instance_name][bandn]["intensity"] = g_settings_get_double(settings, ("intensity-" + bandn).c_str());

    json[section][instance_name][bandn]["mute"] = g_settings_get_boolean(settings, ("mute-" + bandn).c_str()) != 0;

    json[section][instance_name][bandn]["bypass"] = g_settings_get_boolean(settings, ("bypass-" + bandn).c_str()) != 0;
  }
}

void CrystalizerPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  for (int n = 0; n < 13; n++) {
    const auto bandn = "band" + util::to_string(n);

    update_key<double>(json.at(section).at(instance_name)[bandn], settings, "intensity-" + bandn, "intensity");

    update_key<bool>(json.at(section).at(instance_name)[bandn], settings, "mute-" + bandn, "mute");

    update_key<bool>(json.at(section).at(instance_name)[bandn], settings, "bypass-" + bandn, "bypass");
  }
}
