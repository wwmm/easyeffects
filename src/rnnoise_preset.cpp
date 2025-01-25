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

#include "rnnoise_preset.hpp"
#include <gio/gio.h>
#include <glib.h>
#include <nlohmann/json_fwd.hpp>
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"
#include "tags_plugin_name.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

RNNoisePreset::RNNoisePreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::rnnoise::id,
                       tags::schema::rnnoise::input_path,
                       tags::schema::rnnoise::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::rnnoise).append("#").append(util::to_string(index));
}

void RNNoisePreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["model-name"] = util::gsettings_get_string(settings, "model-name");

  json[section][instance_name]["enable-vad"] = g_settings_get_boolean(settings, "enable-vad") != 0;

  json[section][instance_name]["vad-thres"] = g_settings_get_double(settings, "vad-thres");

  json[section][instance_name]["wet"] = g_settings_get_double(settings, "wet");

  json[section][instance_name]["release"] = g_settings_get_double(settings, "release");
}

void RNNoisePreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<bool>(json.at(section).at(instance_name), settings, "enable-vad", "enable-vad");

  update_key<double>(json.at(section).at(instance_name), settings, "vad-thres", "vad-thres");

  update_key<double>(json.at(section).at(instance_name), settings, "wet", "wet");

  update_key<double>(json.at(section).at(instance_name), settings, "release", "release");

  // model-path deprecation
  const auto* model_name_key = "model-name";

  std::string new_model_name =
      json.at(section).at(instance_name).value(model_name_key, get_default<gchar*>(settings, model_name_key));

  if (new_model_name.empty()) {
    const std::string model_path = json.at(section).at(instance_name).value("model-path", "");

    if (!model_path.empty()) {
      new_model_name = std::filesystem::path{model_path}.stem().c_str();

      util::warning("using RNNoise model-path is deprecated, please update your preset; fallback to model-name: " +
                    new_model_name);
    }
  }

  const auto current_model_name = util::gsettings_get_string(settings, model_name_key);

  if (new_model_name != current_model_name) {
    g_settings_set_string(settings, model_name_key, new_model_name.c_str());
  }
}
