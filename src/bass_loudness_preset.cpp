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

#include "bass_loudness_preset.hpp"

BassLoudnessPreset::BassLoudnessPreset() {
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.bassloudness",
                                            "/com/github/wwmm/easyeffects/streaminputs/bassloudness/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.bassloudness",
                                             "/com/github/wwmm/easyeffects/streamoutputs/bassloudness/");
}

void BassLoudnessPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["bass_loudness"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["bass_loudness"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["bass_loudness"]["loudness"] = g_settings_get_double(settings, "loudness");

  json[section]["bass_loudness"]["output"] = g_settings_get_double(settings, "output");

  json[section]["bass_loudness"]["link"] = g_settings_get_double(settings, "link");
}

void BassLoudnessPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("bass_loudness"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("bass_loudness"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("bass_loudness"), settings, "loudness", "loudness");

  update_key<double>(json.at(section).at("bass_loudness"), settings, "output", "output");

  update_key<double>(json.at(section).at("bass_loudness"), settings, "link", "link");
}
