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
#include <sndfile.hh>
#include <string>
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"

namespace presets {

class IrsManager : public QObject {
  Q_OBJECT

 public:
  enum class ImportState { success, no_regular_file, no_frame, unsupported };

  explicit IrsManager(DirectoryManager& directory_manager);
  IrsManager(const IrsManager&) = delete;
  auto operator=(const IrsManager&) -> IrsManager& = delete;
  IrsManager(const IrsManager&&) = delete;
  auto operator=(const IrsManager&&) -> IrsManager& = delete;
  ~IrsManager() override = default;

  int import_impulses(const QList<QString>& url_list);

  static bool remove_impulse_file(const QString& filePath);

  auto get_model() -> ListModel*;

 private:
  DirectoryManager& dir_manager;

  ListModel* model{nullptr};

  QFileSystemWatcher watcher;

  auto import_irs_file(const std::string& file_path) -> ImportState;
};

}  // namespace presets
