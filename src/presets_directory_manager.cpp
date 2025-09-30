/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include "presets_directory_manager.hpp"
#include <qcontainerfwd.h>
#include <qstandardpaths.h>
#include <qtypes.h>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>
#include "config.h"
#include "pipeline_type.hpp"
#include "tags_app.hpp"
#include "util.hpp"

namespace presets {

DirectoryManager::DirectoryManager()
    : app_config_dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()),
      user_input_dir(app_config_dir / "input"),
      user_output_dir(app_config_dir / "output"),
      user_irs_dir(app_config_dir / "irs"),
      user_rnnoise_dir(app_config_dir / "rnnoise"),
      autoload_input_dir(app_config_dir / "autoload/input"),
      autoload_output_dir(app_config_dir / "autoload/output") {
  createUserDirectories();

  /**
   * Initialize input and output directories for community presets.
   * Flatpak specific path (.flatpak-info always present for apps running
   * in the flatpak sandbox).
   */

  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_input.emplace_back("/app/extensions/Presets/input");
    system_data_dir_output.emplace_back("/app/extensions/Presets/output");
    system_data_dir_irs.emplace_back("/app/extensions/Presets/irs");
    system_data_dir_rnnoise.emplace_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths
  for (auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
    dir += dir.endsWith("/") ? "" : "/";

    system_data_dir_input.emplace_back(dir.toStdString() + "input");
    system_data_dir_output.emplace_back(dir.toStdString() + "output");
    system_data_dir_irs.emplace_back(dir.toStdString() + "irs");
    system_data_dir_rnnoise.emplace_back(dir.toStdString() + "rnnoise");
  }
}

// Directory getter implementations
auto DirectoryManager::appConfigDir() const -> const std::filesystem::path& {
  return app_config_dir;
}

auto DirectoryManager::userInputDir() const -> const std::filesystem::path& {
  return user_input_dir;
}

auto DirectoryManager::userOutputDir() const -> const std::filesystem::path& {
  return user_output_dir;
}

auto DirectoryManager::userIrsDir() const -> const std::filesystem::path& {
  return user_irs_dir;
}

auto DirectoryManager::userRnnoiseDir() const -> const std::filesystem::path& {
  return user_rnnoise_dir;
}

auto DirectoryManager::autoloadInputDir() const -> const std::filesystem::path& {
  return autoload_input_dir;
}

auto DirectoryManager::autoloadOutputDir() const -> const std::filesystem::path& {
  return autoload_output_dir;
}

auto DirectoryManager::systemDataDirInput() const -> const std::vector<std::filesystem::path>& {
  return system_data_dir_input;
}

auto DirectoryManager::systemDataDirOutput() const -> const std::vector<std::filesystem::path>& {
  return system_data_dir_output;
}

auto DirectoryManager::systemDataDirIrs() const -> const std::vector<std::filesystem::path>& {
  return system_data_dir_irs;
}

auto DirectoryManager::systemDataDirRnnoise() const -> const std::vector<std::filesystem::path>& {
  return system_data_dir_rnnoise;
}

void DirectoryManager::createUserDirectories() {
  util::create_user_directory(user_input_dir);
  util::create_user_directory(user_output_dir);
  util::create_user_directory(user_irs_dir);
  util::create_user_directory(user_rnnoise_dir);
  util::create_user_directory(autoload_input_dir);
  util::create_user_directory(autoload_output_dir);
}

auto DirectoryManager::getLocalPresetsPaths(PipelineType type) const -> QList<std::filesystem::path> {
  const auto& conf_dir = (type == PipelineType::output) ? user_output_dir : user_input_dir;

  auto it = std::filesystem::directory_iterator{conf_dir};

  return searchPresetsPath(it);
}

auto DirectoryManager::getAutoloadingProfilesPaths(PipelineType type) const -> QList<std::filesystem::path> {
  const auto& conf_dir = (type == PipelineType::output) ? autoload_output_dir : autoload_input_dir;

  auto it = std::filesystem::directory_iterator{conf_dir};

  return searchPresetsPath(it);
}

auto DirectoryManager::getLocalIrsPaths() const -> QList<std::filesystem::path> {
  auto it = std::filesystem::directory_iterator{user_irs_dir};

  return searchPresetsPath(it, irs_ext);
}

auto DirectoryManager::getLocalRnnoisePaths() const -> QList<std::filesystem::path> {
  auto it = std::filesystem::directory_iterator{user_rnnoise_dir};

  return searchPresetsPath(it, rnnoise_ext);
}

auto DirectoryManager::getAllCommunityPresetsPaths(PipelineType type) const -> QList<std::filesystem::path> {
  QList<std::filesystem::path> cp_paths;

  const auto scan_level = 2U;
  const auto& cp_dir_vect = (type == PipelineType::output) ? system_data_dir_output : system_data_dir_input;

  for (const auto& cp_dir : cp_dir_vect) {
    auto cp_fs_path = std::filesystem::path{cp_dir};

    if (!std::filesystem::exists(cp_fs_path)) {
      continue;
    }

    // Scan community package directories for 2 levels
    // (the folder itself and only its subfolders).
    auto it = std::filesystem::directory_iterator{cp_fs_path};

    try {
      while (it != std::filesystem::directory_iterator{}) {
        if (auto package_path = it->path(); std::filesystem::is_directory(it->status())) {
          const auto package_path_name = package_path.string();
          util::debug("scan directory for community presets: " + package_path_name);

          auto package_it = std::filesystem::directory_iterator{package_path};
          const auto sub_cp_vect =
              scanCommunityPackageRecursive(package_it, scan_level, QString::fromStdString(package_path_name));

          cp_paths.append(sub_cp_vect);
        }
        ++it;
      }
    } catch (const std::exception& e) {
      util::warning(e.what());
    }
  }

  return cp_paths;
}

auto DirectoryManager::searchPresetsPath(std::filesystem::directory_iterator& it, const std::string& file_extension)
    -> QList<std::filesystem::path> {
  QList<std::filesystem::path> paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().string() == file_extension) {
        paths.append(it->path());
      }
      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return paths;
}

auto DirectoryManager::scanCommunityPackageRecursive(std::filesystem::directory_iterator& it,
                                                     const uint& top_scan_level,
                                                     const QString& origin) const -> QList<std::filesystem::path> {
  const auto scan_level = top_scan_level - 1U;
  QList<std::filesystem::path> cp_paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().string() == json_ext) {
        cp_paths.append(it->path());
      } else if (scan_level > 0U && std::filesystem::is_directory(it->status())) {
        if (auto path = it->path(); !path.empty()) {
          auto subdir_it = std::filesystem::directory_iterator{path};

          const auto sub_cp_vect = scanCommunityPackageRecursive(
              subdir_it, scan_level, origin + "/" + QString::fromStdString(path.filename().string()));

          cp_paths.append(sub_cp_vect);
        }
      }
      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return cp_paths;
}

}  // namespace presets