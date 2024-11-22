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

#include "presets_manager.hpp"
#include <qfilesystemwatcher.h>
#include <qqml.h>
#include <qstandardpaths.h>
#include <QString>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>
#include "config.h"
#include "preset_type.hpp"
#include "tags_app.hpp"
#include "util.hpp"

namespace presets {

Manager::Manager()
    : app_config_dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()),
      user_input_dir(app_config_dir + "/input"),
      user_output_dir(app_config_dir + "/output"),
      user_irs_dir(app_config_dir + "/irs"),
      user_rnnoise_dir(app_config_dir + "/rnnoise"),
      autoload_input_dir(app_config_dir + "/autoload/input"),
      autoload_output_dir(app_config_dir + "/autoload/output") {
  qmlRegisterSingletonInstance<presets::Manager>("EEpresets", VERSION_MAJOR, VERSION_MINOR, "EEpresetsManager", this);

  // Initialize input and output directories for community presets.
  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox).
  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_input.emplace_back("/app/extensions/Presets/input");
    system_data_dir_output.emplace_back("/app/extensions/Presets/output");
    system_data_dir_irs.emplace_back("/app/extensions/Presets/irs");
    system_data_dir_rnnoise.emplace_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths.
  for (auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
    dir += dir.endsWith("/") ? "" : "/";

    system_data_dir_input.push_back(dir.toStdString() + "input");
    system_data_dir_output.push_back(dir.toStdString() + "output");
    system_data_dir_irs.push_back(dir.toStdString() + "irs");
    system_data_dir_rnnoise.push_back(dir.toStdString() + "rnnoise");
  }

  // create user presets directories

  create_user_directory(user_input_dir);
  create_user_directory(user_output_dir);
  create_user_directory(user_irs_dir);
  create_user_directory(user_rnnoise_dir);
  create_user_directory(autoload_input_dir);
  create_user_directory(autoload_output_dir);

  user_output_watcher.addPath(QString::fromStdString(user_output_dir.string()));

  connect(&user_output_watcher, &QFileSystemWatcher::directoryChanged, [&]() {

  });

  user_input_watcher.addPath(QString::fromStdString(user_input_dir.string()));

  connect(&user_input_watcher, &QFileSystemWatcher::directoryChanged, [&]() {

  });

  autoload_output_watcher.addPath(QString::fromStdString(autoload_output_dir.string()));

  connect(&autoload_output_watcher, &QFileSystemWatcher::directoryChanged, [&]() {

  });

  autoload_input_watcher.addPath(QString::fromStdString(autoload_input_dir.string()));

  connect(&autoload_input_watcher, &QFileSystemWatcher::directoryChanged, [&]() {

  });
}

void Manager::create_user_directory(const std::filesystem::path& path) {
  if (std::filesystem::is_directory(path)) {
    util::debug("user presets directory already exists: " + path.string());

    return;
  }

  if (std::filesystem::create_directories(path)) {
    util::debug("user presets directory created: " + path.string());

    return;
  }

  util::warning("failed to create user presets directory: " + path.string());
}

auto Manager::search_names(std::filesystem::directory_iterator& it) -> std::vector<std::string> {
  std::vector<std::string> names;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().c_str() == json_ext) {
        names.emplace_back(it->path().stem().c_str());
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return names;
}

auto Manager::get_local_presets_name(const PresetType& preset_type) -> std::vector<std::string> {
  const auto conf_dir = (preset_type == PresetType::output) ? user_output_dir : user_input_dir;

  auto it = std::filesystem::directory_iterator{conf_dir};

  auto names = search_names(it);

  return names;
}

}  // namespace presets
