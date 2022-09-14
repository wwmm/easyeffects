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

#pragma once

#include <gio/gio.h>
#include <nlohmann/json.hpp>
#include <string>
#include "preset_type.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

class PluginPresetBase {
 public:
  PluginPresetBase(const char* schema_id,
                   const char* schema_path_input,
                   const char* schema_path_output,
                   PresetType preset_type,
                   const int& index);
  PluginPresetBase(const PluginPresetBase&) = delete;
  auto operator=(const PluginPresetBase&) -> PluginPresetBase& = delete;
  PluginPresetBase(const PluginPresetBase&&) = delete;
  auto operator=(const PluginPresetBase&&) -> PluginPresetBase& = delete;

  virtual ~PluginPresetBase();

  void write(nlohmann::json& json) {
    try {
      save(json);
    } catch (const nlohmann::json::exception& e) {
      util::warning(e.what());
    }
  }

  void read(const nlohmann::json& json) {
    /*
      Old presets do not have the filter instance id.
    */

    if (!json.at(section).contains(instance_name)) {
      instance_name.erase(instance_name.find('#'));
    }

    // For simplicity, exceptions raised while reading presets parameters
    // should be handled outside this method.

    load(json);
  }

 protected:
  int index = 0;

  GSettings* settings = nullptr;

  std::string section, instance_name;

  PresetType preset_type;

  virtual void save(nlohmann::json& json) = 0;

  virtual void load(const nlohmann::json& json) = 0;

  template <typename T>
  auto get_default(GSettings* settings, const std::string& key) -> T {
    GVariant* variant = g_settings_get_default_value(settings, key.c_str());

    T value{};

    if constexpr (std::is_same_v<T, double>) {
      value = g_variant_get_double(variant);
    } else if constexpr (std::is_same_v<T, int>) {
      value = g_variant_get_int32(variant);
    } else if constexpr (std::is_same_v<T, bool>) {
      value = g_variant_get_boolean(variant);
    } else if constexpr (std::is_same_v<T, gchar*>) {
      gsize* length = nullptr;

      value = const_cast<gchar*>(g_variant_get_string(variant, length));
    }

    g_variant_unref(variant);

    return value;
  }

  template <typename T>
  void update_key(const nlohmann::json& json,
                  GSettings* settings,
                  const std::string& key,
                  const std::string& json_key) {
    T new_value;

    if constexpr (!std::is_same_v<T, gchar*>) {
      new_value = json.value(json_key, get_default<T>(settings, key));
    } else {
      std::string tmp = json.value(json_key, get_default<T>(settings, key));
      new_value = g_strdup(tmp.c_str());
    }

    T current_value;

    if constexpr (std::is_same_v<T, double>) {
      current_value = g_settings_get_double(settings, key.c_str());
    } else if constexpr (std::is_same_v<T, int>) {
      current_value = g_settings_get_int(settings, key.c_str());
    } else if constexpr (std::is_same_v<T, bool>) {
      current_value = g_settings_get_boolean(settings, key.c_str());
    } else if constexpr (std::is_same_v<T, gchar*>) {
      current_value = g_settings_get_string(settings, key.c_str());
    }

    if (is_different(current_value, new_value)) {
      if constexpr (std::is_same_v<T, double>) {
        g_settings_set_double(settings, key.c_str(), new_value);
      } else if constexpr (std::is_same_v<T, int>) {
        g_settings_set_int(settings, key.c_str(), new_value);
      } else if constexpr (std::is_same_v<T, bool>) {
        g_settings_set_boolean(settings, key.c_str(), new_value);
      } else if constexpr (std::is_same_v<T, gchar*>) {
        g_settings_set_string(settings, key.c_str(), new_value);
      }
    }

    if constexpr (std::is_same_v<T, gchar*>) {
      g_free(new_value);
      g_free(current_value);
    }
  }

 private:
  /*
    Very naive test for equal values...
  */

  template <typename T>
  auto is_different(const T& a, const T& b) -> bool {
    if constexpr (std::is_same_v<T, gchar*>) {
      return static_cast<bool>(g_strcmp0(a, b) != 0);
    }

    return a != b;
  }
};
