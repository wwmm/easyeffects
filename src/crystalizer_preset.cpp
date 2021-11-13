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

#include "crystalizer_preset.hpp"

CrystalizerPreset::CrystalizerPreset() {
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.crystalizer",
                                            "/com/github/wwmm/easyeffects/streaminputs/crystalizer/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.crystalizer",
                                             "/com/github/wwmm/easyeffects/streamoutputs/crystalizer/");
}

void CrystalizerPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["crystalizer"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["crystalizer"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  for (int n = 0; n < 13; n++) {
    const auto bandn = "band" + std::to_string(n);

    json[section]["crystalizer"][bandn]["intensity"] = g_settings_get_double(settings, ("intensity-" + bandn).c_str());

    json[section]["crystalizer"][bandn]["mute"] = g_settings_get_boolean(settings, ("mute-" + bandn).c_str()) != 0;

    json[section]["crystalizer"][bandn]["bypass"] = g_settings_get_boolean(settings, ("bypass-" + bandn).c_str()) != 0;
  }
}

void CrystalizerPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("crystalizer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("crystalizer"), settings, "output-gain", "output-gain");

  for (int n = 0; n < 13; n++) {
    const auto bandn = "band" + std::to_string(n);

    update_key<double>(json.at(section).at("crystalizer")[bandn], settings, "intensity-" + bandn, "intensity");

    update_key<bool>(json.at(section).at("crystalizer")[bandn], settings, "mute-" + bandn, "mute");

    update_key<bool>(json.at(section).at("crystalizer")[bandn], settings, "bypass-" + bandn, "bypass");
  }
}
