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

#pragma once

#include <qobject.h>
#include <qtmetamacros.h>
#include <filesystem>
#include <string>
#include <vector>

namespace presets {

class Manager : public QObject {
  Q_OBJECT;

 public:
  Manager();
  Manager(const Manager&) = delete;
  auto operator=(const Manager&) -> Manager& = delete;
  Manager(const Manager&&) = delete;
  auto operator=(const Manager&&) -> Manager& = delete;
  ~Manager() override;

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

  const std::string json_ext = ".json";

 signals:
  // signal sending title and description strings
  void presetLoadError(const QString& msg1, const QString& msg2);

 private:
  std::string user_config_dir;

  std::filesystem::path user_input_dir, user_output_dir, user_irs_dir, user_rnnoise_dir, autoload_input_dir,
      autoload_output_dir;

  std::vector<std::string> system_data_dir_input, system_data_dir_output, system_data_dir_irs, system_data_dir_rnnoise;

  static void create_user_directory(const std::filesystem::path& path);
};

}  // namespace presets