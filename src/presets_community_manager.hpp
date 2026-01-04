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

#include <qhashfunctions.h>
#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include "pipeline_type.hpp"
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"

namespace presets {

class CommunityManager : public QObject {
  Q_OBJECT

 public:
  explicit CommunityManager(DirectoryManager& directory_manager);
  CommunityManager(const CommunityManager&) = delete;
  auto operator=(const CommunityManager&) -> CommunityManager& = delete;
  CommunityManager(const CommunityManager&&) = delete;
  auto operator=(const CommunityManager&&) -> CommunityManager& = delete;
  ~CommunityManager() override = default;

  bool import_from_community_package(const PipelineType& pipeline_type,
                                     const QString& file_path,
                                     const QString& package);

  auto scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                        const uint& top_scan_level,
                                        const QString& origin = "") -> QList<std::filesystem::path>;

  auto get_input_model() -> ListModel*;

  auto get_output_model() -> ListModel*;

  void refresh_list_model(const PipelineType& pipeline_type);

  [[nodiscard]] auto getAllCommunityPresetsPaths(PipelineType type) -> QList<std::filesystem::path>;

 private:
  DirectoryManager& dir_manager;

  ListModel* input_model{nullptr};
  ListModel* output_model{nullptr};

  void refreshListModels();

  auto import_addons_from_community_package(const PipelineType& pipeline_type,
                                            const std::filesystem::path& path,
                                            const std::string& package) -> bool;
};

}  // namespace presets
