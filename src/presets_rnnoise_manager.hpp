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

#include <qfilesystemwatcher.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <string>
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"

namespace presets {

class RnnoiseManager : public QObject {
  Q_OBJECT

 public:
  enum class ImportState { success, no_regular_file };

  explicit RnnoiseManager(DirectoryManager& directory_manager);
  RnnoiseManager(const RnnoiseManager&) = delete;
  auto operator=(const RnnoiseManager&) -> RnnoiseManager& = delete;
  RnnoiseManager(const RnnoiseManager&&) = delete;
  auto operator=(const RnnoiseManager&&) -> RnnoiseManager& = delete;
  ~RnnoiseManager() override = default;

  Q_INVOKABLE int import_model(const QList<QString>& url_list);

  Q_INVOKABLE static bool remove_model(const QString& filePath);

  auto get_model() -> ListModel*;

 private:
  DirectoryManager& dir_manager;

  ListModel* model{nullptr};

  QFileSystemWatcher watcher;

  auto import_rnnoise_file(const std::string& file_path) -> ImportState;
};

}  // namespace presets
