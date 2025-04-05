/*
 *  Copyright © 2017-2025 Wellington Wallace
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
#include <gio/gio.h>
#include <nlohmann/json_fwd.hpp>
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"
#include "tags_plugin_name.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

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

  json[section][instance_name]["quick-seek"] = g_settings_get_boolean(settings, "quick-seek") != 0;

  json[section][instance_name]["anti-alias"] = g_settings_get_boolean(settings, "anti-alias") != 0;

  json[section][instance_name]["sequence-length"] = g_settings_get_int(settings, "sequence-length");

  json[section][instance_name]["seek-window"] = g_settings_get_int(settings, "seek-window");

  json[section][instance_name]["overlap-length"] = g_settings_get_int(settings, "overlap-length");

  json[section][instance_name]["tempo-difference"] = g_settings_get_double(settings, "tempo-difference");

  json[section][instance_name]["rate-difference"] = g_settings_get_double(settings, "rate-difference");

  json[section][instance_name]["semitones"] = g_settings_get_double(settings, "semitones");

  // New keys:
  json[section][instance_name]["cents"] = g_settings_get_double(settings, "cents");
  json[section][instance_name]["octaves"] = g_settings_get_double(settings, "octaves");
}

void PitchPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<bool>(json.at(section).at(instance_name), settings, "quick-seek", "quick-seek");

  update_key<bool>(json.at(section).at(instance_name), settings, "anti-alias", "anti-alias");

  update_key<int>(json.at(section).at(instance_name), settings, "sequence-length", "sequence-length");

  update_key<int>(json.at(section).at(instance_name), settings, "seek-window", "seek-window");

  update_key<int>(json.at(section).at(instance_name), settings, "overlap-length", "overlap-length");

  update_key<double>(json.at(section).at(instance_name), settings, "tempo-difference", "tempo-difference");

  update_key<double>(json.at(section).at(instance_name), settings, "rate-difference", "rate-difference");

  update_key<double>(json.at(section).at(instance_name), settings, "semitones", "semitones");

  // New keys:
  update_key<double>(json.at(section).at(instance_name), settings, "cents", "cents");
  update_key<double>(json.at(section).at(instance_name), settings, "octaves", "octaves");
}
