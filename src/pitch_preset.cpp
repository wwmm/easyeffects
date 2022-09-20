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

#include "pitch_preset.hpp"

PitchPreset::PitchPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::pitch::id,
                       tags::schema::pitch::input_path,
                       tags::schema::pitch::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::pitch).append("#").append(util::to_string(index));
}

void PitchPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section][instance_name]["formant"] = util::gsettings_get_string(settings, "formant");

  json[section][instance_name]["transients"] = util::gsettings_get_string(settings, "transients");

  json[section][instance_name]["detector"] = util::gsettings_get_string(settings, "detector");

  json[section][instance_name]["phase"] = util::gsettings_get_string(settings, "phase");

  json[section][instance_name]["cents"] = g_settings_get_int(settings, "cents");

  json[section][instance_name]["semitones"] = g_settings_get_int(settings, "semitones");

  json[section][instance_name]["octaves"] = g_settings_get_int(settings, "octaves");
}

void PitchPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "mode", "mode");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "formant", "formant");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "transients", "transients");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "detector", "detector");

  update_key<gchar*>(json.at(section).at(instance_name), settings, "phase", "phase");

  update_key<int>(json.at(section).at(instance_name), settings, "cents", "cents");

  update_key<int>(json.at(section).at(instance_name), settings, "semitones", "semitones");

  update_key<int>(json.at(section).at(instance_name), settings, "octaves", "octaves");
}
