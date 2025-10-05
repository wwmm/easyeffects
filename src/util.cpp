/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include "util.hpp"
#include <qdebug.h>
#include <qlockfile.h>
#include <qlogging.h>
#include <qstandardpaths.h>
#include <sys/types.h>
#include <QLoggingCategory>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <numbers>
#include <random>
#include <regex>
#include <string>
#include <thread>
#include "spa/utils/dict.h"

namespace util {

static Q_LOGGING_CATEGORY(ee_logs, "easyeffects");

static auto prepare_debug_message(const std::string& message, source_location location) -> std::string {
  auto file_path = std::filesystem::path{location.file_name()};

  std::string msg = file_path.filename().string() + ":" + to_string(location.line()) + "\t" + message;

  return msg;
}

void debug(const std::string& s, source_location location) {
  qCDebug(ee_logs).noquote() << prepare_debug_message(s, location);
}

void fatal(const std::string& s, source_location location) {
  qCFatal(ee_logs).noquote() << prepare_debug_message(s, location);
}

void critical(const std::string& s, source_location location) {
  qCCritical(ee_logs).noquote() << prepare_debug_message(s, location);
}

void warning(const std::string& s, source_location location) {
  qCWarning(ee_logs).noquote() << prepare_debug_message(s, location);
}

void info(const std::string& s, source_location location) {
  qCInfo(ee_logs).noquote() << prepare_debug_message(s, location);
}

void print_thread_id() {
  std::cout << "thread id: " << std::this_thread::get_id() << '\n';
}

void create_user_directory(const std::filesystem::path& path) {
  if (std::filesystem::is_directory(path)) {
    return;
  }

  if (std::filesystem::create_directories(path)) {
    return;
  }

  util::warning(std::format("failed to create directory: {}", path.string()));
}

auto normalize(const double& x, const double& max, const double& min) -> double {
  // Mainly used for gating level bar in gate effects
  return (x - min) / (max - min);
}

auto linear_to_db(const float& amp) -> float {
  if (amp >= minimum_linear_level) {
    return 20.0F * std::log10(amp);
  }

  return minimum_db_level;
}

auto linear_to_db(const double& amp) -> double {
  if (amp >= minimum_linear_d_level) {
    return 20.0 * std::log10(amp);
  }

  return minimum_db_d_level;
}

auto db_to_linear(const float& db) -> float {
  return std::exp((db / 20.0F) * std::numbers::ln10_v<float>);
}

auto db_to_linear(const double& db) -> double {
  return std::exp((db / 20.0) * std::numbers::ln10);
}

auto remove_filename_extension(const std::string& basename) -> std::string {
  return basename.substr(0U, basename.find_last_of('.'));
}

auto random_string(const size_t& length) -> std::string {
  static const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  std::random_device rd;      // Non-deterministic random number generator
  std::mt19937 engine(rd());  // Mersenne Twister engine
  std::uniform_int_distribution<> dist(0, characters.size() - 1);
  std::string result;

  for (size_t i = 0U; i < length; ++i) {
    result += characters[dist(engine)];
  }

  return result;
}

auto search_filename(const std::filesystem::path& path,
                     const std::string& filename,
                     std::string& full_path_result,
                     const uint& top_scan_level) -> bool {
  /**
   * Recursive util to search a filename from an origin full path directory.
   * The search is performed in subdirectories and it's stopped at a specified
   * sublevel (top_scan_level = 1 searches only in the path).
   */

  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    return false;
  }

  const auto fn = path / filename;

  // Get the file in this directory, if exists.
  if (std::filesystem::exists(fn) && std::filesystem::is_regular_file(fn)) {
    // File found, abort the search.
    full_path_result = fn.string();

    return true;
  }

  // The file is not in this directory, search in subdirectories.
  const auto scan_level = top_scan_level - 1U;

  if (scan_level == 0U) {
    return false;
  }

  auto it = std::filesystem::directory_iterator{path};

  try {
    while (it != std::filesystem::directory_iterator{}) {
      if (std::filesystem::is_directory(it->status())) {
        if (const auto p = it->path(); !p.empty()) {
          // Continue the search in the subfolder.
          const auto found = search_filename(p, filename, full_path_result, scan_level);

          if (found) {
            return true;
          }
        }
      }

      ++it;
    }
  } catch (const std::exception& e) {
    util::warning(e.what());

    return false;
  }

  return false;
}

auto compare_versions(const std::string& v0, const std::string& v1) -> int {
  /**
   * This is an util to compare two strings as semver, mainly used to compare
   * two Pipewire versions.
   * The format should adhere to what is defined at `https://semver.org/`.
   * The additional extension label, if present, is ignored and fortunately
   * we don't need to look at it since Pipewire does not use it.
   *
   * Given two version strings v0 and v1, this util returns an integer:
   * - 0 if the versions are equal;
   * - 1 if v0 is higher than v1;
   * - -1 if v0 is lower than v1;
   * - Whichever other number if the comparison fails (i.e. giving one or
   *   both strings not respecting the semver format).
   */

  struct SemVer {
    int major = -1;
    int minor = -1;
    int patch = -1;
  };

  static const auto re_semver = std::regex(R"(^(\d+)\.?(\d+)?\.?(\d+)?)");

  std::array<SemVer, 2> sv{};
  std::array<std::string, 2> v{v0, v1};

  // SemVer struct initialization. Loop the given strings.
  for (int v_idx = 0; v_idx < 2; v_idx++) {
    // For both strings, execute the regular expression search,
    // then loop through the submatches.
    std::smatch match;

    if (!std::regex_search(v[v_idx], match, re_semver)) {
      // The given string is not a semver: the comparison failed.
      return 9;
    }

    // Submatches lookup
    for (int sub_idx = 0, size = match.size(); sub_idx < size; sub_idx++) {
      // Fill the structure converting the string to an integer.
      switch (sub_idx) {
        case 1:  // major
          str_to_num(match[sub_idx].str(), sv[v_idx].major);
          break;

        case 2:  // minor
          str_to_num(match[sub_idx].str(), sv[v_idx].minor);
          break;

        case 3:  // patch
          str_to_num(match[sub_idx].str(), sv[v_idx].patch);
          break;

        default:
          // sub_idx = 0: the first group which is the entire match we don't need.
          break;
      }
    }
  }

  // Now that we are sure to have two valid semver, let's compare each part.
  if (sv[0].major < sv[1].major) {
    return -1;
  } else if (sv[0].major > sv[1].major) {
    return 1;
  }

  if (sv[0].minor < sv[1].minor) {
    return -1;
  } else if (sv[0].minor > sv[1].minor) {
    return 1;
  }

  if (sv[0].patch < sv[1].patch) {
    return -1;
  } else if (sv[0].patch > sv[1].patch) {
    return 1;
  }

  return 0;
}

auto get_lock_file() -> std::unique_ptr<QLockFile> {
  auto lockFile = std::make_unique<QLockFile>(QString::fromStdString(
      QStandardPaths::writableLocation(QStandardPaths::TempLocation).toStdString() + "/easyeffects.lock"));

  lockFile->setStaleLockTime(0);

  bool status = lockFile->tryLock(100);

  if (!status) {
    util::debug("Could not lock the file: " + lockFile->fileName().toStdString());

    switch (lockFile->error()) {
      case QLockFile::NoError:
        break;
      case QLockFile::LockFailedError: {
        util::debug("Another instance already has the lock");
        break;
      }
      case QLockFile::PermissionError: {
        util::debug("No permission to reate the lock file");
        break;
      }
      case QLockFile::UnknownError: {
        util::debug("Unknown error");
        break;
      }
    }
  }

  return lockFile;
}

auto spa_dict_get_bool(const spa_dict* props, const char* key, bool& b) -> bool {
  // Returning bool is for conversion success state.
  // The bool value is assigned to reference parameter.
  if (const auto* v = spa_dict_lookup(props, key)) {
    b = (std::strcmp(v, "true") == 0);

    return true;
  }

  return false;
}

}  // namespace util
