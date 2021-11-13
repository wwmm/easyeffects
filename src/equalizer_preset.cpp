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

#include "equalizer_preset.hpp"

EqualizerPreset::EqualizerPreset()
    : input_settings_left(g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel",
                                                   "/com/github/wwmm/easyeffects/streaminputs/equalizer/leftchannel/")),
      input_settings_right(
          g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel",
                                   "/com/github/wwmm/easyeffects/streaminputs/equalizer/rightchannel/")),
      output_settings_left(
          g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel",
                                   "/com/github/wwmm/easyeffects/streamoutputs/equalizer/leftchannel/")),
      output_settings_right(
          g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel",
                                   "/com/github/wwmm/easyeffects/streamoutputs/equalizer/rightchannel/")) {
  input_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer",
                                            "/com/github/wwmm/easyeffects/streaminputs/equalizer/");

  output_settings = g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer",
                                             "/com/github/wwmm/easyeffects/streamoutputs/equalizer/");
}

EqualizerPreset::~EqualizerPreset() {
  g_object_unref(input_settings_left);
  g_object_unref(input_settings_right);
  g_object_unref(output_settings_left);
  g_object_unref(output_settings_right);
}

void EqualizerPreset::save(nlohmann::json& json, const std::string& section, GSettings* settings) {
  json[section]["equalizer"]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section]["equalizer"]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section]["equalizer"]["mode"] = g_settings_get_string(settings, "mode");

  json[section]["equalizer"]["split-channels"] = g_settings_get_boolean(settings, "split-channels") != 0;

  const auto nbands = g_settings_get_int(settings, "num-bands");

  json[section]["equalizer"]["num-bands"] = nbands;

  if (section == "input") {
    save_channel(json[section]["equalizer"]["left"], input_settings_left, nbands);
    save_channel(json[section]["equalizer"]["right"], input_settings_right, nbands);
  } else if (section == "output") {
    save_channel(json[section]["equalizer"]["left"], output_settings_left, nbands);
    save_channel(json[section]["equalizer"]["right"], output_settings_right, nbands);
  }
}

void EqualizerPreset::save_channel(nlohmann::json& json, GSettings* settings, const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto bandn = "band" + std::to_string(n);

    json[bandn]["type"] = g_settings_get_string(settings, (bandn + "-type").c_str());

    json[bandn]["mode"] = g_settings_get_string(settings, (bandn + "-mode").c_str());

    json[bandn]["slope"] = g_settings_get_string(settings, (bandn + "-slope").c_str());

    json[bandn]["solo"] = g_settings_get_boolean(settings, (bandn + "-solo").c_str()) != 0;

    json[bandn]["mute"] = g_settings_get_boolean(settings, (bandn + "-mute").c_str()) != 0;

    json[bandn]["gain"] = g_settings_get_double(settings, (bandn + "-gain").c_str());

    json[bandn]["frequency"] = g_settings_get_double(settings, (bandn + "-frequency").c_str());

    json[bandn]["q"] = g_settings_get_double(settings, (bandn + "-q").c_str());
  }
}

void EqualizerPreset::load(const nlohmann::json& json, const std::string& section, GSettings* settings) {
  update_key<double>(json.at(section).at("equalizer"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("equalizer"), settings, "output-gain", "output-gain");

  update_string_key(json.at(section).at("equalizer"), settings, "mode", "mode");

  update_key<int>(json.at(section).at("equalizer"), settings, "num-bands", "num-bands");

  update_key<bool>(json.at(section).at("equalizer"), settings, "split-channels", "split-channels");

  const auto nbands = g_settings_get_int(settings, "num-bands");

  if (section == "input") {
    load_channel(json.at(section).at("equalizer").at("left"), input_settings_left, nbands);
    load_channel(json.at(section).at("equalizer").at("right"), input_settings_right, nbands);
  } else if (section == "output") {
    load_channel(json.at(section).at("equalizer").at("left"), output_settings_left, nbands);
    load_channel(json.at(section).at("equalizer").at("right"), output_settings_right, nbands);
  }
}

void EqualizerPreset::load_channel(const nlohmann::json& json, GSettings* settings, const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto bandn = "band" + std::to_string(n);

    update_string_key(json.at(bandn), settings, bandn + "-type", "type");

    update_string_key(json.at(bandn), settings, bandn + "-mode", "mode");

    update_string_key(json.at(bandn), settings, bandn + "-slope", "slope");

    update_key<bool>(json.at(bandn), settings, bandn + "-solo", "solo");

    update_key<bool>(json.at(bandn), settings, bandn + "-mute", "mute");

    update_key<double>(json.at(bandn), settings, bandn + "-gain", "gain");

    update_key<double>(json.at(bandn), settings, bandn + "-frequency", "frequency");

    update_key<double>(json.at(bandn), settings, bandn + "-q", "q");
  }
}
