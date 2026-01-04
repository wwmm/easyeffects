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

#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
#include "pipeline_type.hpp"
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"

namespace presets {

class AutoloadManager : public QObject {
  Q_OBJECT

 public:
  explicit AutoloadManager(DirectoryManager& directory_manager);
  AutoloadManager(const AutoloadManager&) = delete;
  auto operator=(const AutoloadManager&) -> AutoloadManager& = delete;
  AutoloadManager(const AutoloadManager&&) = delete;
  auto operator=(const AutoloadManager&&) -> AutoloadManager& = delete;
  ~AutoloadManager() override = default;

  void add(const PipelineType& pipeline_type,
           const QString& preset_name,
           const QString& device_name,
           const QString& device_description,
           const QString& device_route);

  void remove(const PipelineType& pipeline_type,
              const QString& preset_name,
              const QString& device_name,
              const QString& device_route);

  void load(const PipelineType& pipeline_type, const QString& device_name, const QString& device_route);

  auto getProfiles(const PipelineType& pipeline_type) -> std::vector<nlohmann::json>;

  auto get_input_model() -> ListModel*;

  auto get_output_model() -> ListModel*;

  Q_INVOKABLE void addAutoload(const PipelineType& pipelineType,
                               const QString& presetName,
                               const QString& deviceName,
                               const QString& deviceDescription,
                               const QString& deviceProfile);

  Q_INVOKABLE void removeAutoload(const PipelineType& pipelineType,
                                  const QString& presetName,
                                  const QString& deviceName,
                                  const QString& deviceProfile);

 Q_SIGNALS:
  void loadPresetRequested(const PipelineType& pipeline_type, const QString& preset_name);
  void loadFallbackPresetRequested(const PipelineType& pipeline_type, const QString& preset_name);

 private:
  DirectoryManager& dir_manager;

  ListModel* input_model{nullptr};
  ListModel* output_model{nullptr};

  QFileSystemWatcher input_watcher;
  QFileSystemWatcher output_watcher;

  void prepareFilesystemWatchers();

  void refreshListModels();

  auto find(const PipelineType& pipeline_type, const QString& device_name, const QString& device_route) -> std::string;

  auto getFilePath(const PipelineType& pipeline_type, QString device_name, QString device_route)
      -> std::filesystem::path;
};

}  // namespace presets
