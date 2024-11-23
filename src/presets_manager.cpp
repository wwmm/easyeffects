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
#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qqml.h>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <KLocalizedString>
#include <QString>
#include <exception>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "config.h"
#include "easyeffects_db_streaminputs.h"
#include "easyeffects_db_streamoutputs.h"
#include "preset_type.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
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

auto Manager::get_all_community_presets_paths(const PresetType& preset_type) -> std::vector<std::string> {
  std::vector<std::string> cp_paths;

  const auto scan_level = 2U;

  const auto cp_dir_vect = (preset_type == PresetType::output) ? system_data_dir_output : system_data_dir_input;

  for (const auto& cp_dir : cp_dir_vect) {
    auto cp_fs_path = std::filesystem::path{cp_dir};

    if (!std::filesystem::exists(cp_fs_path)) {
      continue;
    }

    // Scan community package directories for 2 levels (the folder itself and only its subfolders).
    auto it = std::filesystem::directory_iterator{cp_fs_path};

    try {
      while (it != std::filesystem::directory_iterator{}) {
        if (auto package_path = it->path(); std::filesystem::is_directory(it->status())) {
          const auto package_path_name = package_path.string();

          util::debug("scan directory for community presets: " + package_path_name);

          auto package_it = std::filesystem::directory_iterator{package_path};

          /* When C++23 is available, the following line is enough:
          cp_paths.append_range(
              scan_community_package_recursive(package_it, scan_level, package_path_name));
          */

          const auto sub_cp_vect = scan_community_package_recursive(package_it, scan_level, package_path_name);

          cp_paths.insert(cp_paths.end(), sub_cp_vect.cbegin(), sub_cp_vect.cend());
        }

        ++it;
      }
    } catch (const std::exception& e) {
      util::warning(e.what());
    }
  }

  return cp_paths;
}

auto Manager::scan_community_package_recursive(std::filesystem::directory_iterator& it,
                                               const uint& top_scan_level,
                                               const std::string& origin) -> std::vector<std::string> {
  const auto scan_level = top_scan_level - 1U;

  std::vector<std::string> cp_paths;

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_regular_file(it->status()) && it->path().extension().c_str() == json_ext) {
        cp_paths.emplace_back(origin + "/" + it->path().stem().c_str());
      } else if (scan_level > 0U && std::filesystem::is_directory(it->status())) {
        if (auto path = it->path(); !path.empty()) {
          auto subdir_it = std::filesystem::directory_iterator{path};

          /* When C++23 is available, the following line is enough:
          cp_paths.append_range(
              scan_community_package_recursive(subdir_it, scan_level, origin + "/" + path.filename().c_str()));
          */

          const auto sub_cp_vect =
              scan_community_package_recursive(subdir_it, scan_level, origin + "/" + path.filename().c_str());

          cp_paths.insert(cp_paths.end(), sub_cp_vect.cbegin(), sub_cp_vect.cend());
        }
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());
  }

  return cp_paths;
}

auto Manager::get_community_preset_info(const PresetType& preset_type,
                                        const std::string& path) -> std::pair<std::string, std::string> {
  // Given the full path of a community preset, extract and return:
  // 1. the preset name
  // 2. the package name

  static const auto re_pack_name = std::regex(R"(^\/([^/]+))");
  static const auto re_preset_name = std::regex(R"([^/]+$)");

  const auto cp_dir_vect = (preset_type == PresetType::output) ? system_data_dir_output : system_data_dir_input;

  for (const auto& cp_dir : cp_dir_vect) {
    // In order to extract the package name, let's check if
    // the preset belongs to the selected system data directory.

    const auto cp_dir_size = cp_dir.size();

    // Skip if the lenght of the path is shorter than `cp_dir + "/"`.
    if (path.size() <= cp_dir_size + 1U) {
      continue;
    }

    // Check if the preset is contained in the selected system data directory.
    // starts_with gets a string_view, so we use the version with character array.
    if (!path.starts_with(cp_dir.c_str())) {
      continue;
    }

    // Extract the package name.
    std::smatch pack_match;

    // Get the subdirectory/package.
    // - Full match: "/pack_name"
    // - Group 1: "pack_name"
    std::regex_search(path.cbegin() + cp_dir_size, path.cend(), pack_match, re_pack_name);

    // Skip if the match failed.
    if (pack_match.size() != 2U) {
      continue;
    }

    // Extract the preset name.
    std::smatch name_match;

    std::regex_search(path.cbegin() + cp_dir_size, path.cend(), name_match, re_preset_name);

    // Skip if the match failed.
    if (name_match.size() != 1U) {
      continue;
    }

    return std::make_pair(name_match.str(0), pack_match.str(1));
  }

  util::warning("can't extract info strings for the community preset: " + path);

  // Placeholders in case of issues
  return std::make_pair(i18n("Community Preset").toStdString(), i18n("Package").toStdString());
}

void Manager::save_blocklist(const PresetType& preset_type, nlohmann::json& json) {
  std::vector<std::string> blocklist;

  switch (preset_type) {
    case PresetType::output: {
      const auto list = db::StreamOutputs::blocklist();

      for (const auto& l : list) {
        blocklist.push_back(l.toStdString());
      }

      json["output"]["blocklist"] = blocklist;

      break;
    }
    case PresetType::input: {
      const auto list = db::StreamInputs::blocklist();

      for (const auto& l : list) {
        blocklist.push_back(l.toStdString());
      }

      json["input"]["blocklist"] = blocklist;

      break;
    }
  }
}

auto Manager::load_blocklist(const PresetType& preset_type, const nlohmann::json& json) -> bool {
  std::vector<std::string> blocklist;

  switch (preset_type) {
    case PresetType::input: {
      try {
        auto list = json.at("input").at("blocklist").get<std::vector<std::string>>();

        auto new_list = QStringList();

        for (const auto& app : list) {
          new_list.append(QString::fromStdString(app));
        }

        db::StreamInputs::setBlocklist(new_list);
      } catch (const nlohmann::json::exception& e) {
        db::StreamInputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_format);

        util::warning(e.what());

        return false;
      } catch (...) {
        db::StreamInputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_generic);

        return false;
      }

      break;
    }
    case PresetType::output: {
      try {
        auto list = json.at("output").at("blocklist").get<std::vector<std::string>>();

        auto new_list = QStringList();

        for (const auto& app : list) {
          new_list.append(QString::fromStdString(app));
        }

        db::StreamOutputs::setBlocklist(new_list);
      } catch (const nlohmann::json::exception& e) {
        db::StreamOutputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_format);

        util::warning(e.what());

        return false;
      } catch (...) {
        db::StreamOutputs::setBlocklist(QStringList{});

        notify_error(PresetError::blocklist_generic);

        return false;
      }

      break;
    }
  }

  return true;
}

void Manager::notify_error(const PresetError& preset_error, const std::string& plugin_name) {
  QString plugin_translated;

  try {
    const auto base_name = tags::plugin_name::Model::self().getBaseName(QString::fromStdString(plugin_name));
    plugin_translated = tags::plugin_name::Model::self().translate(base_name) + ": ";
  } catch (std::out_of_range& e) {
    util::debug(e.what());
  }

  switch (preset_error) {
    case PresetError::blocklist_format: {
      util::warning(
          "A parsing error occurred while trying to load the blocklist from the preset. The file could be invalid "
          "or corrupted. Please check its content.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"), i18n("Wrong Format in Excluded Apps List"));

      break;
    }
    case PresetError::blocklist_generic: {
      util::warning("A generic error occurred while trying to load the blocklist from the preset.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"),
                             i18n("Generic Error While Loading Excluded Apps List"));

      break;
    }
    case PresetError::pipeline_format: {
      util::warning(
          "A parsing error occurred while trying to load the pipeline from the preset. The file could be invalid "
          "or corrupted. Please check its content.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"), i18n("Wrong Format in Effects List"));

      break;
    }
    case PresetError::pipeline_generic: {
      util::warning("A generic error occurred while trying to load the pipeline from the preset.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"), i18n("Generic Error While Loading Effects List"));

      break;
    }
    case PresetError::plugin_format: {
      util::warning("A parsing error occurred while trying to load the " + plugin_name +
                    " plugin from the preset. The file could be invalid or "
                    "corrupted. Please check its content.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"),
                             plugin_translated + i18n("One or More Parameters Have a Wrong Format"));

      break;
    }
    case PresetError::plugin_generic: {
      util::warning("A generic error occurred while trying to load the " + plugin_name + " plugin from the preset.");

      Q_EMIT presetLoadError(i18n("Preset Not Loaded Correctly"),
                             plugin_translated + i18n("Generic Error While Loading The Effect"));

      break;
    }
    default:
      break;
  }
}

}  // namespace presets
