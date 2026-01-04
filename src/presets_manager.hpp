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
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <vector>
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_autoload_manager.hpp"
#include "presets_community_manager.hpp"
#include "presets_directory_manager.hpp"
#include "presets_irs_manager.hpp"
#include "presets_list_model.hpp"
#include "presets_rnnoise_manager.hpp"

namespace presets {

class Manager : public QObject {
  Q_OBJECT

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

  auto preset_file_exists(const PipelineType& pipeline_type, const std::string& name) -> bool;

  void autoload(const PipelineType& pipeline_type, const QString& device_name, const QString& device_route);

  auto get_local_presets_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path>;

  Q_INVOKABLE bool add(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool savePresetFile(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool remove(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool renameLocalPresetFile(const PipelineType& pipeline_type,
                                         const QString& name,
                                         const QString& newName);

  Q_INVOKABLE bool loadLocalPresetFile(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool importPresets(const PipelineType& pipeline_type, const QList<QString>& url_list);

  Q_INVOKABLE bool exportPresets(const PipelineType& pipeline_type, const QString& dirUrl);

  Q_INVOKABLE int importImpulses(const QList<QString>& url_list);

  Q_INVOKABLE int importRNNoiseModel(const QList<QString>& url_list);

  Q_INVOKABLE static bool removeImpulseFile(const QString& filePath);

  Q_INVOKABLE static bool removeRNNoiseModel(const QString& filePath);

  Q_INVOKABLE void refreshCommunityPresets(const PipelineType& pipeline_type);

  Q_INVOKABLE bool loadCommunityPresetFile(const PipelineType& pipeline_type,
                                           const QString& file_path,
                                           const QString& package_name);

  Q_INVOKABLE bool importFromCommunityPackage(const PipelineType& pipeline_type,
                                              const QString& file_path,
                                              const QString& package);

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
  // signal sending title and description strings
  void presetLoadError(const QString& msg1, const QString& msg2);

 private:
  DirectoryManager dir_manager;

  AutoloadManager autoload_manager{dir_manager};

  CommunityManager community_manager{dir_manager};

  IrsManager irs_manager{dir_manager};

  RnnoiseManager rnnoise_manager{dir_manager};

  QFileSystemWatcher user_output_watcher, user_input_watcher;

  ListModel *outputListModel, *inputListModel;

  void initialize_qml_types();

  void refresh_list_models();

  void prepare_filesystem_watchers();

  static void write_plugins_preset(const PipelineType& pipeline_type, const QStringList& plugins, nlohmann::json& json);

  auto read_effects_pipeline_from_preset(const PipelineType& pipeline_type,
                                         const std::filesystem::path& input_file,
                                         nlohmann::json& json,
                                         std::vector<std::string>& plugins) -> bool;

  auto read_plugins_preset(const PipelineType& pipeline_type,
                           const std::vector<std::string>& plugins,
                           const nlohmann::json& json) -> bool;

  void prepare_last_used_preset_key(const PipelineType& pipeline_type);

  static void save_blocklist(const PipelineType& pipeline_type, nlohmann::json& json);

  auto load_blocklist(const PipelineType& pipeline_type, const nlohmann::json& json) -> bool;

  static void set_last_preset_keys(const PipelineType& pipeline_type,
                                   const QString& preset_name = "",
                                   const QString& package_name = "");

  auto load_preset_file(const PipelineType& pipeline_type, const std::filesystem::path& input_file) -> bool;

  void notify_error(const PresetError& preset_error, const std::string& plugin_name = "");

  static auto create_wrapper(const PipelineType& pipeline_type, const QString& filter_name)
      -> std::optional<std::unique_ptr<PluginPresetBase>>;

  void update_used_presets_list(const PipelineType& pipeline_type, const QString& name);
};

}  // namespace presets
