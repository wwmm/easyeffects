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

#pragma once

#include <gio/gio.h>
#include <sigc++/signal.h>
#include <filesystem>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "plugin_preset_base.hpp"
#include "preset_type.hpp"

class PresetsManager {
 public:
  PresetsManager();
  PresetsManager(const PresetsManager&) = delete;
  auto operator=(const PresetsManager&) -> PresetsManager& = delete;
  PresetsManager(const PresetsManager&&) = delete;
  auto operator=(const PresetsManager&&) -> PresetsManager& = delete;
  ~PresetsManager();

  enum class PresetError {
    blocklist_format,
    blocklist_generic,
    pipeline_format,
    pipeline_generic,
    plugin_format,
    plugin_generic
  };

  const std::string json_ext = ".json";

  // signal sending title and description strings
  sigc::signal<void(const std::string, const std::string)> preset_load_error;

  auto get_all_community_presets_paths(const PresetType& preset_type) -> std::vector<std::string>;

  auto scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                        const uint& top_scan_level,
                                        const std::string& origin = "") -> std::vector<std::string>;

  auto get_community_preset_info(const PresetType& preset_type, const std::string& path)
      -> std::pair<std::string, std::string>;

  auto get_local_presets_name(const PresetType& preset_type) -> std::vector<std::string>;

  auto search_names(std::filesystem::directory_iterator& it) -> std::vector<std::string>;

  void add(const PresetType& preset_type, const std::string& name);

  void save_preset_file(const PresetType& preset_type, const std::string& name);

  auto get_loaded_preset(const PresetType& preset_type) -> std::string;

  static void write_plugins_preset(const PresetType& preset_type,
                                   const std::vector<std::string>& plugins,
                                   nlohmann::json& json);

  void remove(const PresetType& preset_type, const std::string& name);

  auto load_local_preset_file(const PresetType& preset_type, const std::string& name) -> bool;

  auto load_community_preset_file(const PresetType& preset_type,
                                  const std::string& full_path_stem,
                                  const std::string& package_name) -> bool;

  auto read_effects_pipeline_from_preset(const PresetType& preset_type,
                                         const std::filesystem::path& input_file,
                                         nlohmann::json& json,
                                         std::vector<std::string>& plugins) -> bool;

  auto read_plugins_preset(const PresetType& preset_type,
                           const std::vector<std::string>& plugins,
                           const nlohmann::json& json) -> bool;

  void import_from_filesystem(const PresetType& preset_type, const std::string& file_path);

  void import_from_community_package(const PresetType& preset_type,
                                     const std::string& file_path,
                                     const std::string& package);

  void add_autoload(const PresetType& preset_type,
                    const std::string& preset_name,
                    const std::string& device_name,
                    const std::string& device_description,
                    const std::string& device_profile);

  void remove_autoload(const PresetType& preset_type,
                       const std::string& preset_name,
                       const std::string& device_name,
                       const std::string& device_profile);

  auto find_autoload(const PresetType& preset_type, const std::string& device_name, const std::string& device_profile)
      -> std::string;

  void autoload(const PresetType& preset_type, const std::string& device_name, const std::string& device_profile);

  auto get_autoload_profiles(const PresetType& preset_type) -> std::vector<nlohmann::json>;

  auto preset_file_exists(const PresetType& preset_type, const std::string& name) -> bool;

  sigc::signal<void(const std::string& preset_name)> user_output_preset_created;
  sigc::signal<void(const std::string& preset_name)> user_output_preset_removed;
  sigc::signal<void(const std::string& preset_name)> user_input_preset_created;
  sigc::signal<void(const std::string& preset_name)> user_input_preset_removed;

  sigc::signal<void(const std::vector<nlohmann::json>& profiles)> autoload_input_profiles_changed;
  sigc::signal<void(const std::vector<nlohmann::json>& profiles)> autoload_output_profiles_changed;

 private:
  std::string user_config_dir;

  std::filesystem::path user_input_dir, user_output_dir, user_irs_dir, user_rnnoise_dir, autoload_input_dir,
      autoload_output_dir;

  std::vector<std::string> system_data_dir_input, system_data_dir_output, system_data_dir_irs, system_data_dir_rnnoise;

  GSettings *settings = nullptr, *soe_settings = nullptr, *sie_settings = nullptr;

  GFileMonitor *user_output_monitor = nullptr, *user_input_monitor = nullptr;

  GFileMonitor *autoload_output_monitor = nullptr, *autoload_input_monitor = nullptr;

  static void create_user_directory(const std::filesystem::path& path);

  auto import_addons_from_community_package(const PresetType& preset_type,
                                            const std::filesystem::path& path,
                                            const std::string& package) -> bool;

  void set_last_preset_keys(const PresetType& preset_type,
                            const std::string& preset_name = "",
                            const std::string& package_name = "");

  auto load_preset_file(const PresetType& preset_type, const std::filesystem::path& input_file) -> bool;

  void save_blocklist(const PresetType& preset_type, nlohmann::json& json);

  auto load_blocklist(const PresetType& preset_type, const nlohmann::json& json) -> bool;

  void notify_error(const PresetError& preset_error, const std::string& plugin_name = "");

  static auto create_wrapper(const PresetType& preset_type, std::string_view filter_name)
      -> std::optional<std::unique_ptr<PluginPresetBase>>;
};
