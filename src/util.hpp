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

#include <qlockfile.h>
#include <spa/utils/dict.h>
#include <sys/types.h>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <mutex>
#include <source_location>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

namespace util {
constexpr float minimum_db_level = -100.0F;
constexpr double minimum_db_d_level = -100.0;
constexpr float minimum_linear_level = 0.00001F;
constexpr double minimum_linear_d_level = 0.00001;

using source_location = std::source_location;

void debug(const std::string& s, source_location location = source_location::current());
void fatal(const std::string& s, source_location location = source_location::current());
void critical(const std::string& s, source_location location = source_location::current());
void warning(const std::string& s, source_location location = source_location::current());
void info(const std::string& s, source_location location = source_location::current());

void create_user_directory(const std::filesystem::path& path);

auto copy_all_files(const std::filesystem::path& source_dir, const std::filesystem::path& target_dir) -> int;

auto normalize(const double& x, const double& max, const double& min = 1.0) -> double;

auto linear_to_db(const float& amp) -> float;
auto linear_to_db(const double& amp) -> double;

auto db_to_linear(const float& db) -> float;
auto db_to_linear(const double& db) -> double;

auto remove_filename_extension(const std::string& basename) -> std::string;

void print_thread_id();

auto compare_versions(const std::string& v0, const std::string& v1) -> int;

auto random_string(const size_t& length) -> std::string;

auto search_filename(const std::filesystem::path& path,
                     const std::string& filename,
                     std::string& full_path_result,
                     const uint& top_scan_level = std::numeric_limits<uint>::max()) -> bool;

auto get_lock_file() -> std::unique_ptr<QLockFile>;

auto spa_dict_get_bool(const spa_dict* props, const char* key, bool& b) -> bool;

auto interpolate(const std::vector<double>& x_source,
                 const std::vector<double>& y_source,
                 const std::vector<double>& x_new) -> std::vector<double>;

inline std::mutex& fftw_lock() {
  static std::mutex fftw_mutex;
  return fftw_mutex;
}

template <typename T>
void print_type(T v) {
  warning(typeid(v).name());
}

template <typename T>
auto str_to_num(const std::string& str, T& num) -> bool {
  // This is a more robust implementation of `std::from_chars`
  // so that we don't have to do every time with `std::from_chars_result` structure.
  // We don't care of error types, so a simple bool is returned on success/fail.
  // A left trim is performed on strings so that the conversion could success
  // even if there are leading whitespaces and/or the plus sign.

  auto first_char = str.find_first_not_of(" +\n\r\t\v\f");

  if (first_char == std::string::npos) {
    return false;
  }

#ifndef ENABLE_LIBCPP_WORKAROUNDS

  const auto result = std::from_chars(str.data() + first_char, str.data() + str.size(), num);

  return (result.ec == std::errc());

#else

  if constexpr (std::is_floating_point_v<T>) {
    char* endp = nullptr;

    // We're asking for C locale which is preallocated, so no alloc here.

    auto loc = newlocale(LC_ALL_MASK, "C", nullptr);

    if constexpr (std::is_same_v<T, float>) {
      num = strtof_l(str.data() + first_char, &endp, loc);
    } else {
      num = strtod_l(str.data() + first_char, &endp, loc);
    }

    // We gotta "free" it anyway.

    freelocale(loc);

    return (endp && !*endp && (endp != (str.data() + first_char)));
  } else {
    const auto result = std::from_chars(str.data() + first_char, str.data() + str.size(), num);

    return (result.ec == std::errc());
  }

#endif
}

template <typename T>
auto to_string(const T& num, const std::string def = "0") -> std::string {
  /**
   * This is used to replace `std::to_string` as a locale independent
   * number conversion using `std::to_chars`.
   * An additional string parameter could be eventually provided with a
   * default value to return in case the conversion fails.
   *
   * Max buffer length:
   * number of base-10 digits that can be represented by the type T without
   * change + number of base-10 digits that are necessary to uniquely represent
   * all distinct values of the type T (meaningful only for real numbers) +
   * room for other characters such as "+-e,."
   */
  const size_t max = std::numeric_limits<T>::digits10 + std::numeric_limits<T>::max_digits10 + 10U;

  std::array<char, max> buffer;

  const auto p_init = buffer.data();

  const auto result = std::to_chars(p_init, p_init + max, num);

  return (result.ec == std::errc()) ? std::string(p_init, result.ptr - p_init) : def;
}

template <class T>
concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;

template <Number T>
auto logspace(const T& start, const T& stop, const uint& npoints) -> std::vector<T> {
  std::vector<T> output;

  if (stop <= start || npoints < 2) {
    return output;
  }

  auto log10_start = std::log10(start);
  auto log10_stop = std::log10(stop);

  const T delta = (log10_stop - log10_start) / static_cast<T>(npoints - 1);

  output.push_back(start);

  T v = log10_start;

  while (output.size() < npoints - 1) {
    v += delta;

    if constexpr (std::is_same_v<T, float>) {
      output.push_back(std::pow(10.0F, v));
    } else {
      output.push_back(std::pow(10.0, v));
    }
  }

  output.push_back(stop);

  return output;
}

template <Number T>
auto linspace(const T& start, const T& stop, const uint& npoints) -> std::vector<T> {
  std::vector<T> output;

  if (stop <= start || npoints < 2) {
    return output;
  }

  const T delta = (stop - start) / static_cast<T>(npoints - 1);

  output.push_back(start);

  T v = start;

  while (output.size() < npoints - 1) {
    v += delta;

    output.push_back(v);
  }

  output.push_back(stop);

  return output;
}

template <typename T>
auto spa_dict_get_string(const spa_dict* props, const char* key, T& str) -> bool {
  // If we will use string views in the future, this template could be useful.
  if (const auto* s = spa_dict_lookup(props, key)) {
    str = s;

    return true;
  }

  return false;
}

template <typename T>
auto spa_dict_get_num(const spa_dict* props, const char* key, T& num) -> bool {
  if (const auto* n = spa_dict_lookup(props, key)) {
    return util::str_to_num(std::string(n), num);
  }

  return false;
}

template <typename T>
void copy_bulk(std::vector<T>& input, std::vector<T>& output) {
  std::copy_n(input.begin(), output.size(), output.begin());

  std::move(input.begin() + output.size(), input.end(), input.begin());

  input.resize(input.size() - output.size());
}

template <typename T>
void copy_bulk(std::vector<T>& input, std::span<T>& output) {
  std::copy_n(input.begin(), output.size(), output.begin());

  std::move(input.begin() + output.size(), input.end(), input.begin());

  input.resize(input.size() - output.size());
}

}  // namespace util
