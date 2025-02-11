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

#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <vector>
#include "pipeline_type.hpp"
#include "plugin_preset_base.hpp"
#include "presets_list_model.hpp"

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

  enum class ImpulseImportState { success, no_regular_file, no_frame, no_stereo };

  constexpr static std::string json_ext = ".json";

  constexpr static std::string irs_ext = ".irs";

  auto preset_file_exists(const PipelineType& pipeline_type, const std::string& name) -> bool;

  void autoload(const PipelineType& pipeline_type, const QString& device_name, const QString& device_profile);

  Q_INVOKABLE bool add(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool savePresetFile(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool remove(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool loadLocalPresetFile(const PipelineType& pipeline_type, const QString& name);

  Q_INVOKABLE bool importPresets(const PipelineType& pipeline_type, const QList<QString>& url_list);

  Q_INVOKABLE int importImpulses(const QList<QString>& url_list);

  Q_INVOKABLE static void removeImpulseFile(const QString& filePath);

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

 signals:
  // signal sending title and description strings
  void presetLoadError(const QString& msg1, const QString& msg2);

 private:
  std::string app_config_dir;

  std::filesystem::path user_input_dir, user_output_dir, user_irs_dir, user_rnnoise_dir, autoload_input_dir,
      autoload_output_dir;

  std::vector<std::string> system_data_dir_input, system_data_dir_output, system_data_dir_irs, system_data_dir_rnnoise;

  QFileSystemWatcher user_output_watcher, user_input_watcher, autoload_output_watcher, autoload_input_watcher,
      irs_watcher;

  ListModel *outputListModel, *inputListModel, *communityOutputListModel, *communityInputListModel,
      *autoloadingOutputListmodel, *autoloadingInputListmodel, *irsListModel;

  static void create_user_directory(const std::filesystem::path& path);

  auto get_local_presets_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path>;

  auto get_local_irs_paths() -> QList<std::filesystem::path>;

  auto get_autoloading_profiles_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path>;

  auto get_all_community_presets_paths(const PipelineType& pipeline_type) -> QList<std::filesystem::path>;

  static void refresh_list_models(ListModel* model, std::function<QList<std::filesystem::path>()> get_paths);

  void prepare_filesystem_watchers();

  static void write_plugins_preset(const PipelineType& pipeline_type, const QStringList& plugins, nlohmann::json& json);

  auto read_effects_pipeline_from_preset(const PipelineType& pipeline_type,
                                         const std::filesystem::path& input_file,
                                         nlohmann::json& json,
                                         std::vector<std::string>& plugins) -> bool;

  auto read_plugins_preset(const PipelineType& pipeline_type,
                           const std::vector<std::string>& plugins,
                           const nlohmann::json& json) -> bool;

  auto find_autoload(const PipelineType& pipeline_type, const QString& device_name, const QString& device_profile)
      -> std::string;

  auto get_autoload_profiles(const PipelineType& pipeline_type) -> std::vector<nlohmann::json>;

  void prepare_last_used_preset_key(const PipelineType& pipeline_type);

  static auto search_presets_path(std::filesystem::directory_iterator& it, const std::string& file_extension = json_ext)
      -> QList<std::filesystem::path>;

  auto scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                        const uint& top_scan_level,
                                        const QString& origin = "") -> QList<std::filesystem::path>;

  static void save_blocklist(const PipelineType& pipeline_type, nlohmann::json& json);

  auto load_blocklist(const PipelineType& pipeline_type, const nlohmann::json& json) -> bool;

  auto import_addons_from_community_package(const PipelineType& pipeline_type,
                                            const std::filesystem::path& path,
                                            const std::string& package) -> bool;

  static void set_last_preset_keys(const PipelineType& pipeline_type,
                                   const QString& preset_name = "",
                                   const QString& package_name = "");

  auto load_preset_file(const PipelineType& pipeline_type, const std::filesystem::path& input_file) -> bool;

  void notify_error(const PresetError& preset_error, const std::string& plugin_name = "");

  static auto create_wrapper(const PipelineType& pipeline_type, const QString& filter_name)
      -> std::optional<std::unique_ptr<PluginPresetBase>>;

  auto import_irs_file(const std::string& file_path) -> ImpulseImportState;
};

}  // namespace presets
