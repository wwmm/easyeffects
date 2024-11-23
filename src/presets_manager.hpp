/*
 *  Copyright © 2017-2024 Wellington Wallace
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

#include <qfilesystemwatcher.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>
#include "preset_type.hpp"

namespace presets {

class Manager : public QObject {
  Q_OBJECT;

 public:
  Manager();
  Manager(const Manager&) = delete;
  auto operator=(const Manager&) -> Manager& = delete;
  Manager(const Manager&&) = delete;
  auto operator=(const Manager&&) -> Manager& = delete;
  ~Manager() override = default;

  static Manager& self() {
    static Manager pm;
    return pm;
  }

  enum class PresetError {
    blocklist_format,
    blocklist_generic,
    pipeline_format,
    pipeline_generic,
    plugin_format,
    plugin_generic
  };

  const std::string json_ext = ".json";

  auto search_names(std::filesystem::directory_iterator& it) -> std::vector<std::string>;

  auto get_local_presets_name(const PresetType& preset_type) -> std::vector<std::string>;

  auto get_all_community_presets_paths(const PresetType& preset_type) -> std::vector<std::string>;

  auto get_community_preset_info(const PresetType& preset_type,
                                 const std::string& path) -> std::pair<std::string, std::string>;

  auto load_local_preset_file(const PresetType& preset_type, const std::string& name) -> bool;

  auto load_community_preset_file(const PresetType& preset_type,
                                  const std::string& full_path_stem,
                                  const std::string& package_name) -> bool;

  auto find_autoload(const PresetType& preset_type,
                     const std::string& device_name,
                     const std::string& device_profile) -> std::string;

  void add_autoload(const PresetType& preset_type,
                    const std::string& preset_name,
                    const std::string& device_name,
                    const std::string& device_description,
                    const std::string& device_profile);

  void remove_autoload(const PresetType& preset_type,
                       const std::string& preset_name,
                       const std::string& device_name,
                       const std::string& device_profile);

  void autoload(const PresetType& preset_type, const std::string& device_name, const std::string& device_profile);

  auto get_autoload_profiles(const PresetType& preset_type) -> std::vector<nlohmann::json>;

  auto preset_file_exists(const PresetType& preset_type, const std::string& name) -> bool;

 signals:
  // signal sending title and description strings
  void presetLoadError(const QString& msg1, const QString& msg2);

 private:
  std::string app_config_dir;

  std::filesystem::path user_input_dir, user_output_dir, user_irs_dir, user_rnnoise_dir, autoload_input_dir,
      autoload_output_dir;

  std::vector<std::string> system_data_dir_input, system_data_dir_output, system_data_dir_irs, system_data_dir_rnnoise;

  QFileSystemWatcher user_output_watcher, user_input_watcher, autoload_output_watcher, autoload_input_watcher;

  static void create_user_directory(const std::filesystem::path& path);

  auto scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                        const uint& top_scan_level,
                                        const std::string& origin = "") -> std::vector<std::string>;

  static void save_blocklist(const PresetType& preset_type, nlohmann::json& json);

  auto load_blocklist(const PresetType& preset_type, const nlohmann::json& json) -> bool;

  static void set_last_preset_keys(const PresetType& preset_type,
                                   const std::string& preset_name = "",
                                   const std::string& package_name = "");

  auto load_preset_file(const PresetType& preset_type, const std::filesystem::path& input_file) -> bool;

  void notify_error(const PresetError& preset_error, const std::string& plugin_name = "");
};

}  // namespace presets