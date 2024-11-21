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
#include <qstandardpaths.h>
#include <filesystem>
#include "tags_app.hpp"
#include "util.hpp"

namespace presets {

Manager::Manager()
    : user_config_dir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString()),
      user_input_dir(user_config_dir + "/easyeffects/input"),
      user_output_dir(user_config_dir + "/easyeffects/output"),
      user_irs_dir(user_config_dir + "/easyeffects/irs"),
      user_rnnoise_dir(user_config_dir + "/easyeffects/rnnoise"),
      autoload_input_dir(user_config_dir + "/easyeffects/autoload/input"),
      autoload_output_dir(user_config_dir + "/easyeffects/autoload/output") {
  // Initialize input and output directories for community presets.
  // Flatpak specific path (.flatpak-info always present for apps running in the flatpak sandbox).
  if (std::filesystem::is_regular_file(tags::app::flatpak_info_file)) {
    system_data_dir_input.emplace_back("/app/extensions/Presets/input");
    system_data_dir_output.emplace_back("/app/extensions/Presets/output");
    system_data_dir_irs.emplace_back("/app/extensions/Presets/irs");
    system_data_dir_rnnoise.emplace_back("/app/extensions/Presets/rnnoise");
  }

  // Regular paths.
  //   for (const gchar* const* xdg_data_dirs = g_get_system_data_dirs(); *xdg_data_dirs != nullptr;) {
  for (const auto& dir : QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation)) {
    // dir += dir.endsWith("/") ? "" : "/";

    util::warning(dir.toStdString());

    // system_data_dir_input.push_back(dir + "easyeffects/input");
    // system_data_dir_output.push_back(dir + "easyeffects/output");
    // system_data_dir_irs.push_back(dir + "easyeffects/irs");
    // system_data_dir_rnnoise.push_back(dir + "easyeffects/rnnoise");
  }

  // create user presets directories

  create_user_directory(user_input_dir);
  create_user_directory(user_output_dir);
  create_user_directory(user_irs_dir);
  create_user_directory(user_rnnoise_dir);
  create_user_directory(autoload_input_dir);
  create_user_directory(autoload_output_dir);
}

Manager::~Manager() {}

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

}  // namespace presets
