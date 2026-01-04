/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <qlist.h>
#include <qtypes.h>
#include <QStandardPaths>
#include <QString>
#include <filesystem>
#include <string>
#include <vector>
#include "pipeline_type.hpp"

namespace presets {

class DirectoryManager {
 public:
  explicit DirectoryManager();
  DirectoryManager(const DirectoryManager&) = delete;
  auto operator=(const DirectoryManager&) -> DirectoryManager& = delete;
  DirectoryManager(const DirectoryManager&&) = delete;
  auto operator=(const DirectoryManager&&) -> DirectoryManager& = delete;
  ~DirectoryManager() = default;

  [[nodiscard]] auto appConfigDir() const -> const std::filesystem::path&;
  [[nodiscard]] auto userInputDir() const -> const std::filesystem::path&;
  [[nodiscard]] auto userOutputDir() const -> const std::filesystem::path&;
  [[nodiscard]] auto userIrsDir() const -> const std::filesystem::path&;
  [[nodiscard]] auto userRnnoiseDir() const -> const std::filesystem::path&;
  [[nodiscard]] auto autoloadInputDir() const -> const std::filesystem::path&;
  [[nodiscard]] auto autoloadOutputDir() const -> const std::filesystem::path&;

  [[nodiscard]] auto systemDataDirInput() const -> const std::vector<std::filesystem::path>&;
  [[nodiscard]] auto systemDataDirOutput() const -> const std::vector<std::filesystem::path>&;
  [[nodiscard]] auto systemDataDirIrs() const -> const std::vector<std::filesystem::path>&;
  [[nodiscard]] auto systemDataDirRnnoise() const -> const std::vector<std::filesystem::path>&;

  [[nodiscard]] auto getLocalPresetsPaths(PipelineType type) const -> QList<std::filesystem::path>;
  [[nodiscard]] auto getAutoloadProfilesPaths(PipelineType type) const -> QList<std::filesystem::path>;
  [[nodiscard]] auto getLocalIrsPaths() const -> QList<std::filesystem::path>;
  [[nodiscard]] auto getLocalRnnoisePaths() const -> QList<std::filesystem::path>;

  auto scanDirectoryRecursive(std::filesystem::directory_iterator& it,
                              const uint& top_scan_level,
                              const QString& origin,
                              const std::string& file_extension = json_ext) const -> QList<std::filesystem::path>;

  static constexpr auto json_ext = ".json";
  static constexpr auto* irs_ext = ".irs";
  static constexpr auto* sofa_ext = ".sofa";
  static constexpr auto* rnnoise_ext = ".rnnn";

 private:
  // Core directory paths
  std::filesystem::path app_config_dir;
  std::filesystem::path app_data_dir;
  std::filesystem::path user_input_dir;
  std::filesystem::path user_output_dir;
  std::filesystem::path user_irs_dir;
  std::filesystem::path user_rnnoise_dir;
  std::filesystem::path autoload_input_dir;
  std::filesystem::path autoload_output_dir;

  // System data directories
  std::vector<std::filesystem::path> system_data_dir_input;
  std::vector<std::filesystem::path> system_data_dir_output;
  std::vector<std::filesystem::path> system_data_dir_irs;
  std::vector<std::filesystem::path> system_data_dir_rnnoise;

  void createUserDirectories();

  static auto searchPresetsPath(std::filesystem::directory_iterator& it,
                                const std::vector<std::string>& file_extensions) -> QList<std::filesystem::path>;

  void xdg_migration();
};

}  // namespace presets
