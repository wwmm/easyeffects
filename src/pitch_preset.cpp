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

#include "pitch_preset.hpp"

PitchPreset::PitchPreset() {
  input_settings =
      g_settings_new_with_path("com.github.wwmm.easyeffects.pitch", "/com/github/wwmm/easyeffects/streaminputs/pitch/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.pitch",
                                             "/com/github/wwmm/easyeffects/streamoutputs/pitch/");
}

void PitchPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["pitch"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["pitch"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["pitch"]["mode"] = util::gsettings_get_string(settings, "mode");

  json[section]["pitch"]["formant"] = util::gsettings_get_string(settings, "formant");

  json[section]["pitch"]["transients"] = util::gsettings_get_string(settings, "transients");

  json[section]["pitch"]["detector"] = util::gsettings_get_string(settings, "detector");

  json[section]["pitch"]["phase"] = util::gsettings_get_string(settings, "phase");

  json[section]["pitch"]["cents"] = g_settings_get_int(settings, "cents");

  json[section]["pitch"]["semitones"] = g_settings_get_int(settings, "semitones");

  json[section]["pitch"]["octaves"] = g_settings_get_int(settings, "octaves");
}

void PitchPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("pitch"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("pitch"), settings, "output-gain", "output-gain");

  update_key<gchar*>(json.at(section).at("pitch"), settings, "mode", "mode");

  update_key<gchar*>(json.at(section).at("pitch"), settings, "formant", "formant");

  update_key<gchar*>(json.at(section).at("pitch"), settings, "transients", "transients");

  update_key<gchar*>(json.at(section).at("pitch"), settings, "detector", "detector");

  update_key<gchar*>(json.at(section).at("pitch"), settings, "phase", "phase");

  update_key<int>(json.at(section).at("pitch"), settings, "semitones", "semitones");

  update_key<int>(json.at(section).at("pitch"), settings, "octaves", "octaves");
}
