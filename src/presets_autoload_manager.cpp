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

#include "presets_autoload_manager.hpp"
#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
#include "easyeffects_db.h"
#include "pipeline_type.hpp"
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"
#include "util.hpp"

namespace presets {

AutoloadManager::AutoloadManager(DirectoryManager& directory_manager)
    : dir_manager(directory_manager),
      input_model(new ListModel(this, ListModel::ModelType::Autoload)),
      output_model(new ListModel(this, ListModel::ModelType::Autoload)) {
  refreshListModels();
  prepareFilesystemWatchers();
}

auto AutoloadManager::get_input_model() -> ListModel* {
  return input_model;
}

auto AutoloadManager::get_output_model() -> ListModel* {
  return output_model;
}

void AutoloadManager::prepareFilesystemWatchers() {
  input_watcher.addPath(QString::fromStdString(dir_manager.autoloadInputDir().string()));
  output_watcher.addPath(QString::fromStdString(dir_manager.autoloadOutputDir().string()));

  connect(&input_watcher, &QFileSystemWatcher::directoryChanged,
          [&]() { input_model->update(dir_manager.getAutoloadProfilesPaths(PipelineType::input)); });

  connect(&output_watcher, &QFileSystemWatcher::directoryChanged,
          [&]() { output_model->update(dir_manager.getAutoloadProfilesPaths(PipelineType::output)); });
}

void AutoloadManager::refreshListModels() {
  input_model->update(dir_manager.getAutoloadProfilesPaths(PipelineType::input));
  output_model->update(dir_manager.getAutoloadProfilesPaths(PipelineType::output));
}

auto AutoloadManager::getFilePath(const PipelineType& pipeline_type, QString device_name, QString device_route)
    -> std::filesystem::path {
  device_name.replace("/", "_");
  device_route.replace("/", "_");  // pipewire can put "/" in the route description

  switch (pipeline_type) {
    case PipelineType::output:
      return dir_manager.autoloadOutputDir() /
             std::filesystem::path{device_name.toStdString() + ":" + device_route.toStdString() +
                                   DirectoryManager::json_ext};
    case PipelineType::input:
      return dir_manager.autoloadInputDir() /
             std::filesystem::path{device_name.toStdString() + ":" + device_route.toStdString() +
                                   DirectoryManager::json_ext};
  }

  return {};
}

void AutoloadManager::add(const PipelineType& pipeline_type,
                          const QString& preset_name,
                          const QString& device_name,
                          const QString& device_description,
                          const QString& device_route) {
  nlohmann::json json;

  auto path = getFilePath(pipeline_type, device_name, device_route);

  bool already_exists = std::filesystem::exists(path);

  std::ofstream o(path);

  json["device"] = device_name.toStdString();
  json["device-description"] = device_description.toStdString();
  json["device-profile"] = device_route.toStdString();
  json["preset-name"] = preset_name.toStdString();

  o << std::setw(4) << json << '\n';

  util::debug(std::format("Added autoload preset file: {}", path.string()));

  o.close();

  if (already_exists) {
    auto* model = pipeline_type == PipelineType::input ? input_model : output_model;

    model->emit_data_changed(path);
  }
}

void AutoloadManager::remove(const PipelineType& pipeline_type,
                             const QString& preset_name,
                             const QString& device_name,
                             const QString& device_route) {
  auto path = getFilePath(pipeline_type, device_name, device_route);

  if (!std::filesystem::is_regular_file(path)) {
    return;
  }

  nlohmann::json json;

  std::ifstream is(path);

  is >> json;

  if (preset_name.toStdString() == json.value("preset-name", "") &&
      device_route.toStdString() == json.value("device-profile", "")) {
    std::filesystem::remove(path);

    util::debug(std::format("Removed autoload: {}", path.string()));
  }
}

auto AutoloadManager::find(const PipelineType& pipeline_type, const QString& device_name, const QString& device_route)
    -> std::string {
  auto path = getFilePath(pipeline_type, device_name, device_route);

  if (!std::filesystem::is_regular_file(path)) {
    return "";
  }

  nlohmann::json json;

  std::ifstream is(path);

  is >> json;

  return json.value("preset-name", "");
}

void AutoloadManager::load(const PipelineType& pipeline_type, const QString& device_name, const QString& device_route) {
  const auto name = find(pipeline_type, device_name, device_route);

  if (name.empty()) {
    bool use_fallback = pipeline_type == PipelineType::input ? DbMain::inputAutoloadingUsesFallback()
                                                             : DbMain::outputAutoloadingUsesFallback();

    util::debug(
        std::format("Could not find an autoload profile for the device {} and route {}. And the user did not enable "
                    "the fallback preset. No autoload will be done.",
                    device_name.toStdString(), device_route.toStdString()));

    if (!use_fallback) {
      return;
    }

    util::debug(std::format(
        "Could not find an autoload profile for the device {} and route {}. We will try to load the fallback preset.",
        device_name.toStdString(), device_route.toStdString()));

    QString fallback = pipeline_type == PipelineType::input ? DbMain::inputAutoloadingFallbackPreset()
                                                            : DbMain::outputAutoloadingFallbackPreset();

    if (!fallback.isEmpty()) {
      util::debug(
          std::format("Autoload fallback preset {} for device {}", fallback.toStdString(), device_name.toStdString()));

      Q_EMIT loadPresetRequested(pipeline_type, fallback);

      return;
    }

    util::debug(std::format(
        "The {} fallback preset name is empty. No preset will be autoloaded for the device {} and route {}",
        pipeline_type == PipelineType::input ? "input" : "output", fallback.toStdString(), device_name.toStdString()));

    return;
  }

  util::debug(std::format("Autoload local preset {} for device {}", name, device_name.toStdString()));

  Q_EMIT loadPresetRequested(pipeline_type, QString::fromStdString(name));
}

auto AutoloadManager::getProfiles(const PipelineType& pipeline_type) -> std::vector<nlohmann::json> {
  std::filesystem::path autoload_dir;
  std::vector<nlohmann::json> list;

  switch (pipeline_type) {
    case PipelineType::output:
      autoload_dir = dir_manager.autoloadOutputDir();
      break;
    case PipelineType::input:
      autoload_dir = dir_manager.autoloadInputDir();
      break;
  }

  auto it = std::filesystem::directory_iterator{autoload_dir};

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status())) {
        if (it->path().extension().string() == DirectoryManager::json_ext) {
          nlohmann::json json;

          std::ifstream is(autoload_dir / it->path());

          is >> json;

          list.push_back(json);
        }
      }

      ++it;
    }

    return list;
  } catch (const std::exception& e) {
    util::warning(e.what());

    return list;
  }
}

void AutoloadManager::addAutoload(const PipelineType& pipelineType,
                                  const QString& presetName,
                                  const QString& deviceName,
                                  const QString& deviceDescription,
                                  const QString& deviceProfile) {
  add(pipelineType, presetName, deviceName, deviceDescription, deviceProfile);
}

void AutoloadManager::removeAutoload(const PipelineType& pipelineType,
                                     const QString& presetName,
                                     const QString& deviceName,
                                     const QString& deviceProfile) {
  remove(pipelineType, presetName, deviceName, deviceProfile);
}

}  // namespace presets
