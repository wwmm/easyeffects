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

#include "presets_irs_manager.hpp"
#include <mysofa.h>
#include <qcontainerfwd.h>
#include <qfilesystemwatcher.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qurl.h>
#include <filesystem>
#include <format>
#include <sndfile.hh>
#include <string>
#include "presets_directory_manager.hpp"
#include "presets_list_model.hpp"
#include "util.hpp"

namespace presets {

IrsManager::IrsManager(DirectoryManager& directory_manager)
    : dir_manager(directory_manager), model(new ListModel(this, ListModel::ModelType::IRS)) {
  model->update(dir_manager.getLocalIrsPaths());

  watcher.addPath(QString::fromStdString(dir_manager.userIrsDir().string()));

  connect(&watcher, &QFileSystemWatcher::directoryChanged, [&]() { model->update(dir_manager.getLocalIrsPaths()); });
}

auto IrsManager::get_model() -> ListModel* {
  return model;
}

auto IrsManager::import_irs_file(const std::string& file_path) -> ImportState {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(std::format("{} is not a file!", p.string()));

    return ImportState::no_regular_file;
  }

  int error = 0;
  struct MYSOFA_HRTF* sofa = mysofa_load(file_path.c_str(), &error);

  bool is_sofa = error == MYSOFA_OK && sofa != nullptr;

  if (sofa != nullptr) {
    mysofa_free(sofa);
  }

  if (!is_sofa) {
    auto file = SndfileHandle(file_path);

    if (file.frames() == 0) {
      util::warning("Cannot import the impulse response! The format may be corrupted or unsupported.");
      util::warning(std::format("{} loading failed", file_path));

      return ImportState::no_frame;
    }

    if (file.channels() != 1 && file.channels() != 2 && file.channels() != 4) {
      util::warning("Only mono, stereo and true stereo impulse files are supported!");
      util::warning(std::format("{} loading failed", file_path));

      return ImportState::unsupported;
    }
  }

  auto base_name = p.stem().string();

  auto extension =
      (p.extension().string() == DirectoryManager::sofa_ext) ? DirectoryManager::sofa_ext : DirectoryManager::irs_ext;

  auto out_path = dir_manager.userIrsDir() / (base_name + extension);

  // Collision Handling: Check if file exists and increment suffix

  int counter = 1;

  while (std::filesystem::exists(out_path)) {
    // Generates names like "filename (1).irs", "filename (2).irs", etc.

    out_path = dir_manager.userIrsDir() / std::format("{} ({}){}", base_name, counter++, extension);
  }

  std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

  util::debug(std::format("Irs file successfully imported to: {}", out_path.string()));

  return ImportState::success;
}

int IrsManager::import_impulses(const QList<QString>& url_list) {
  for (const auto& u : url_list) {
    auto url = QUrl(u);

    if (url.isLocalFile()) {
      auto path = std::filesystem::path{url.toLocalFile().toStdString()};

      if (auto import_state = import_irs_file(path); import_state != ImportState::success) {
        return static_cast<int>(import_state);
      }
    }
  }

  return static_cast<int>(ImportState::success);
}

bool IrsManager::remove_impulse_file(const QString& filePath) {
  bool result = false;

  if (std::filesystem::exists(filePath.toStdString())) {
    result = std::filesystem::remove(filePath.toStdString());
  }

  if (result) {
    util::debug(std::format("Removed irs file: {}", filePath.toStdString()));
  } else {
    util::warning(std::format("Failed to removed the irs file: {}", filePath.toStdString()));
  }

  return result;
}

}  // namespace presets
