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

#ifndef PLUGIN_PRESET_BASE_HPP
#define PLUGIN_PRESET_BASE_HPP

#include <gio/gio.h>
#include <nlohmann/json.hpp>
#include <string>
#include "preset_type.hpp"
#include "util.hpp"

class PluginPresetBase {
 public:
  PluginPresetBase() = default;
  PluginPresetBase(const PluginPresetBase&) = delete;
  auto operator=(const PluginPresetBase&) -> PluginPresetBase& = delete;
  PluginPresetBase(const PluginPresetBase&&) = delete;
  auto operator=(const PluginPresetBase&&) -> PluginPresetBase& = delete;

  virtual ~PluginPresetBase() {
    g_object_unref(input_settings);
    g_object_unref(output_settings);
  };

  void write(PresetType preset_type, nlohmann::json& json) {
    try {
      switch (preset_type) {
        case PresetType::output:
          save(json, "output", output_settings);
          break;
        case PresetType::input:
          save(json, "input", input_settings);
          break;
      }
    } catch (const nlohmann::json::exception& e) {
      util::warning(e.what());
    }
  }

  void read(PresetType preset_type, const nlohmann::json& json) {
    try {
      switch (preset_type) {
        case PresetType::output:
          load(json, "output", output_settings);
          break;
        case PresetType::input:
          load(json, "input", input_settings);
          break;
      }
    } catch (const nlohmann::json::exception& e) {
      util::warning(e.what());
    }
  }

 protected:
  GSettings *input_settings = nullptr, *output_settings = nullptr;

  virtual void save(nlohmann::json& json, const std::string& section, GSettings* settings) = 0;

  virtual void load(const nlohmann::json& json, const std::string& section, GSettings* settings) = 0;

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

      value = g_variant_get_string(variant, length);
    }

    g_variant_unref(variant);

    return value;
  }

  template <typename T>
  void update_key(const nlohmann::json& json,
                  GSettings* settings,
                  const std::string& key,
                  const std::string& json_key) {
    const T new_value = json.value(json_key, get_default<T>(settings, key));

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
  }

  void update_string_key(const nlohmann::json& json,
                         GSettings* settings,
                         const Glib::ustring& key,
                         const std::string& json_key) {
    // const auto& current_value = settings->get_string(key);

    // const Glib::ustring& new_value = json.value(json_key, get_default<std::string>(settings, key));

    // if (current_value != new_value) {
    //   settings->set_string(key, new_value);
    // }
  }

 private:
  /*
    Very naive test for equal values...
  */

  template <typename T>
  auto is_different(const T& a, const T& b) -> bool {
    if constexpr (std::is_same_v<T, gchar*>) {
      return static_cast<bool>(g_strcmp0(a, b) == 0);
    }

    return a != b;
  }
};

#endif
