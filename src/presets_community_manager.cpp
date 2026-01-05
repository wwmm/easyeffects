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

#include "presets_community_manager.hpp"
#include <qcontainerfwd.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
#include "pipeline_type.hpp"
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace presets {

CommunityManager::CommunityManager(DirectoryManager& directory_manager)
    : dir_manager(directory_manager),
      input_model(new ListModel(this, ListModel::ModelType::Community)),
      output_model(new ListModel(this, ListModel::ModelType::Community)) {
  refreshListModels();
}

auto CommunityManager::get_input_model() -> ListModel* {
  return input_model;
}

auto CommunityManager::get_output_model() -> ListModel* {
  return output_model;
}

void CommunityManager::refresh_list_model(const PipelineType& pipeline_type) {
  switch (pipeline_type) {
    case PipelineType::input: {
      input_model->update(getAllCommunityPresetsPaths(PipelineType::input));
      break;
    }
    case PipelineType::output: {
      output_model->update(getAllCommunityPresetsPaths(PipelineType::output));
      break;
    }
  }
}

void CommunityManager::refreshListModels() {
  input_model->update(getAllCommunityPresetsPaths(PipelineType::input));
  output_model->update(getAllCommunityPresetsPaths(PipelineType::output));
}

auto CommunityManager::scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                                        const uint& top_scan_level,
                                                        const QString& origin) -> QList<std::filesystem::path> {
  const auto scan_level = top_scan_level - 1U;

  QList<std::filesystem::path> cp_paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) &&
          it->path().extension().string() == DirectoryManager::json_ext) {
        cp_paths.append(it->path());
      } else if (scan_level > 0U && std::filesystem::is_directory(it->status())) {
        if (auto path = it->path(); !path.empty()) {
          auto subdir_it = std::filesystem::directory_iterator{path};

          const auto sub_cp_vect = scan_community_package_recursive(
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

auto CommunityManager::import_addons_from_community_package(const PipelineType& pipeline_type,
                                                            const std::filesystem::path& path,
                                                            const std::string& package) -> bool {
  /**
   * Here we parse the json community preset in order to import the list of
   * addons:
   * 1. Convolver Impulse Response Files
   * 2. RNNoise Models
   */

  // This method assumes that the path is valid and package string is not empty,
  // their check has already been made in import_from_community_package();
  std::ifstream is(path);

  nlohmann::json json;

  const auto irs_ext = ".irs";
  const auto rnnn_ext = ".rnnn";

  try {
    is >> json;

    std::vector<std::string> conv_irs;
    std::vector<std::string> rn_models;

    const auto* pt_key = (pipeline_type == PipelineType::output) ? "output" : "input";

    // Fill conv_irs and rn_models vectors extracting the addon names from
    // the json preset and append the respective file extension.
    for (const auto& plugin : json.at(pt_key).at("plugins_order").get<std::vector<std::string>>()) {
      if (plugin.starts_with(tags::plugin_name::BaseName::convolver.toStdString())) {
        conv_irs.push_back(json.at(pt_key).at(plugin).at("kernel-name").get<std::string>() + irs_ext);
      }

      if (plugin.starts_with(tags::plugin_name::BaseName::rnnoise.toStdString())) {
        rn_models.push_back(json.at(pt_key).at(plugin).at("model-name").get<std::string>() + rnnn_ext);
      }
    }

    // For every filename of both vectors, search the full path and copy the file locally.
    for (const auto& irs_name : conv_irs) {
      std::string path;

      bool found = false;

      for (auto xdg_dir : dir_manager.systemDataDirIrs()) {
        xdg_dir.append("/");
        xdg_dir.append(package);

        if (util::search_filename(std::filesystem::path{xdg_dir}, irs_name, path, 3U)) {
          const auto out_path = std::filesystem::path{dir_manager.userIrsDir()} / irs_name;

          std::filesystem::copy_file(path, out_path, std::filesystem::copy_options::overwrite_existing);

          util::debug(std::format("Successfully imported community preset addon {} locally", irs_name));

          found = true;

          break;
        }
      }

      if (!found) {
        util::warning(std::format("Community preset addon {} not found!", irs_name));

        return false;
      }
    }

    for (const auto& model_name : rn_models) {
      std::string path;

      bool found = false;

      for (auto xdg_dir : dir_manager.systemDataDirRnnoise()) {
        xdg_dir.append("/");
        xdg_dir.append(package);

        if (util::search_filename(std::filesystem::path{xdg_dir}, model_name, path, 3U)) {
          const auto out_path = std::filesystem::path{dir_manager.userRnnoiseDir()} / model_name;

          std::filesystem::copy_file(path, out_path, std::filesystem::copy_options::overwrite_existing);

          util::debug(std::format("Successfully imported community preset addon {} locally", model_name));

          found = true;

          break;
        }
      }

      if (!found) {
        util::warning(std::format("Community preset addon {} not found!", model_name));

        return false;
      }
    }

    return true;
  } catch (const std::exception& e) {
    util::warning(e.what());

    return false;
  }
}

bool CommunityManager::import_from_community_package(const PipelineType& pipeline_type,
                                                     const QString& file_path,
                                                     const QString& package) {
  // When importing presets from a community package, we do NOT overwrite
  // the local preset if it has the same name.

  std::filesystem::path p{file_path.toStdString()};

  if (!std::filesystem::exists(p) || package.isEmpty()) {
    util::warning(std::format("{} does not exist! Please reload the community preset list", p.string()));

    return false;
  }

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(std::format("{} is not a file! Please reload the community preset list", p.string()));

    return false;
  }

  if (p.extension().string() != DirectoryManager::json_ext) {
    return false;
  }

  bool preset_can_be_copied = false;

  // We limit the max copy attempts in order to not flood the local directory
  // if the user keeps clicking the import button.
  uint i = 0U;

  static const auto max_copy_attempts = 10;

  const auto conf_dir =
      (pipeline_type == PipelineType::output) ? dir_manager.userOutputDir() : dir_manager.userInputDir();

  std::filesystem::path out_path;

  try {
    do {
      // In case of destination file already existing, we try to append
      // an incremental numeric suffix.
      const auto suffix = (i == 0U) ? "" : "-" + util::to_string(i);

      out_path = conf_dir.string() + "/" + p.stem().string() + suffix + DirectoryManager::json_ext;

      if (!std::filesystem::exists(out_path)) {
        preset_can_be_copied = true;

        break;
      }
    } while (++i < max_copy_attempts);
  } catch (const std::exception& e) {
    util::warning(std::format("Can't import the community preset: {}", p.string()));

    util::warning(e.what());

    return false;
  }

  if (!preset_can_be_copied) {
    util::warning(std::format("Can't import the community preset: {}", p.string()));

    util::warning("Exceeded the maximum copy attempts; please delete or rename your local preset");

    return false;
  }

  // Now we know that the preset is OK to be copied, but we first check for addons.
  if (!import_addons_from_community_package(pipeline_type, p, package.toStdString())) {
    util::warning(
        std::format("Can't import addons for the community preset: {}; Import stage aborted, please reload the "
                    "community preset list",
                    p.string()));

    util::warning("If the issue goes on, contact the maintainer of the community package");

    return false;
  }

  std::filesystem::copy_file(p, out_path);

  util::debug(std::format("Successfully imported the community preset to: {}", out_path.string()));

  return true;
}

auto CommunityManager::getAllCommunityPresetsPaths(PipelineType type) -> QList<std::filesystem::path> {
  QList<std::filesystem::path> cp_paths;

  const auto scan_level = 2U;
  const auto& cp_dir_vect =
      (type == PipelineType::output) ? dir_manager.systemDataDirOutput() : dir_manager.systemDataDirInput();

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
          util::debug(std::format("Scan directory for community presets: {}", package_path_name));

          auto package_it = std::filesystem::directory_iterator{package_path};
          const auto sub_cp_vect =
              dir_manager.scanDirectoryRecursive(package_it, scan_level, QString::fromStdString(package_path_name));

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

}  // namespace presets
