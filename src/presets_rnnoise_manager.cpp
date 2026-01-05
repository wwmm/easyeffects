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

#include "presets_rnnoise_manager.hpp"
#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qurl.h>
#include <filesystem>
#include <format>
#include <string>
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"
#include "util.hpp"

namespace presets {

RnnoiseManager::RnnoiseManager(DirectoryManager& directory_manager)
    : dir_manager(directory_manager), model(new ListModel(this, ListModel::ModelType::RNNOISE)) {
  model->update(dir_manager.getLocalRnnoisePaths());

  watcher.addPath(QString::fromStdString(dir_manager.userRnnoiseDir().string()));

  connect(&watcher, &QFileSystemWatcher::directoryChanged,
          [&]() { model->update(dir_manager.getLocalRnnoisePaths()); });
}

auto RnnoiseManager::get_model() -> ListModel* {
  return model;
}

auto RnnoiseManager::import_rnnoise_file(const std::string& file_path) -> ImportState {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(std::format("{} is not a file!", p.string()));

    return ImportState::no_regular_file;
  }

  auto out_path = dir_manager.userRnnoiseDir() / p.filename();

  out_path.replace_extension(DirectoryManager::rnnoise_ext);

  std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

  util::debug(std::format("Irs file successfully imported to: {}", out_path.string()));

  return ImportState::success;
}

int RnnoiseManager::import_model(const QList<QString>& url_list) {
  for (const auto& u : url_list) {
    auto url = QUrl(u);

    if (url.isLocalFile()) {
      auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (auto import_state = import_rnnoise_file(path); import_state != ImportState::success) {
        return static_cast<int>(import_state);
      }
    }
  }

  return static_cast<int>(ImportState::success);
}

bool RnnoiseManager::remove_model(const QString& filePath) {
  bool result = false;

  if (std::filesystem::exists(filePath.toStdString())) {
    result = std::filesystem::remove(filePath.toStdString());
  }

  if (result) {
    util::debug(std::format("Removed the rnnoise model: {}", filePath.toStdString()));
  } else {
    util::warning(std::format("Failed to remove the rnnoise model: {}", filePath.toStdString()));
  }

  return result;
}

}  // namespace presets
