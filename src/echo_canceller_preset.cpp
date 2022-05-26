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

#include "echo_canceller_preset.hpp"

EchoCancellerPreset::EchoCancellerPreset() {
  input_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streaminputs/echocanceller/").c_str());

  output_settings = g_settings_new_with_path(preset_id.c_str(), (tags::app::path + "/streamoutputs/echocanceller/").c_str());
}

void EchoCancellerPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["echo_canceller"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["echo_canceller"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["echo_canceller"]["frame-size"] = g_settings_get_int(settings, "frame-size");

  json[section]["echo_canceller"]["filter-length"] = g_settings_get_int(settings, "filter-length");
}

void EchoCancellerPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("echo_canceller"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("echo_canceller"), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at("echo_canceller"), settings, "frame-size", "frame-size");

  update_key<int>(json.at(section).at("echo_canceller"), settings, "filter-length", "filter-length");
}
