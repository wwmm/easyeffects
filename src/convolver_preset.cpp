/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "convolver_preset.hpp"

ConvolverPreset::ConvolverPreset(PresetType preset_type, const int& index) : PluginPresetBase(preset_type, index) {
  switch (preset_type) {
    case PresetType::input:
      settings = g_settings_new_with_path(tags::schema::convolver::id, tags::schema::convolver::input_path);
      break;
    case PresetType::output:
      settings = g_settings_new_with_path(tags::schema::convolver::id, tags::schema::convolver::output_path);
      break;
  }
}

void ConvolverPreset::save(nlohmann::json& json) {
  json[section]["convolver"]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section]["convolver"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["convolver"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["convolver"]["kernel-path"] = util::gsettings_get_string(settings, "kernel-path");

  json[section]["convolver"]["ir-width"] = g_settings_get_int(settings, "ir-width");

  json[section]["convolver"]["autogain"] = g_settings_get_boolean(settings, "autogain") != 0;
}

void ConvolverPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at("convolver"), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at("convolver"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("convolver"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("convolver"), settings, "kernel-path", "kernel-path");

  update_key<int>(json.at(section).at("convolver"), settings, "ir-width", "ir-width");

  update_key<bool>(json.at(section).at("convolver"), settings, "autogain", "autogain");
}
