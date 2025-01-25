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

#include "convolver_preset.hpp"
#include <gio/gio.h>
#include <glib.h>
#include <nlohmann/json_fwd.hpp>
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"
#include "tags_plugin_name.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

ConvolverPreset::ConvolverPreset(PresetType preset_type, const int& index)
    : PluginPresetBase(tags::schema::convolver::id,
                       tags::schema::convolver::input_path,
                       tags::schema::convolver::output_path,
                       preset_type,
                       index) {
  instance_name.assign(tags::plugin_name::convolver).append("#").append(util::to_string(index));
}

void ConvolverPreset::save(nlohmann::json& json) {
  json[section][instance_name]["bypass"] = g_settings_get_boolean(settings, "bypass") != 0;

  json[section][instance_name]["input-gain"] = g_settings_get_double(settings, "input-gain");

  json[section][instance_name]["output-gain"] = g_settings_get_double(settings, "output-gain");

  json[section][instance_name]["kernel-name"] = util::gsettings_get_string(settings, "kernel-name");

  json[section][instance_name]["ir-width"] = g_settings_get_int(settings, "ir-width");

  json[section][instance_name]["autogain"] = g_settings_get_boolean(settings, "autogain") != 0;
}

void ConvolverPreset::load(const nlohmann::json& json) {
  update_key<bool>(json.at(section).at(instance_name), settings, "bypass", "bypass");

  update_key<double>(json.at(section).at(instance_name), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at(instance_name), settings, "output-gain", "output-gain");

  update_key<int>(json.at(section).at(instance_name), settings, "ir-width", "ir-width");

  update_key<bool>(json.at(section).at(instance_name), settings, "autogain", "autogain");

  // kernel-path deprecation
  const auto* kernel_name_key = "kernel-name";

  std::string new_kernel_name =
      json.at(section).at(instance_name).value(kernel_name_key, get_default<gchar*>(settings, kernel_name_key));

  if (new_kernel_name.empty()) {
    const std::string kernel_path = json.at(section).at(instance_name).value("kernel-path", "");

    if (!kernel_path.empty()) {
      new_kernel_name = std::filesystem::path{kernel_path}.stem().c_str();

      util::warning("using Convolver kernel-path is deprecated, please update your preset; fallback to kernel-name: " +
                    new_kernel_name);
    }
  }

  const auto current_kernel_name = util::gsettings_get_string(settings, kernel_name_key);

  if (new_kernel_name != current_kernel_name) {
    g_settings_set_string(settings, kernel_name_key, new_kernel_name.c_str());
  }
}
