/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <gdk/gdk.h>
#include <glib-object.h>
#include <glib.h>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <limits>
#include <ranges>
#include <source_location>
#include <string>
#include <thread>
#include <vector>

namespace util {

const float minimum_db_level = -100.0F;
const double minimum_db_d_level = -100.0;
const float minimum_linear_level = 0.00001F;
const double minimum_linear_d_level = 0.00001;

void debug(const std::string& s, std::source_location location = std::source_location::current());
void error(const std::string& s, std::source_location location = std::source_location::current());
void critical(const std::string& s, std::source_location location = std::source_location::current());
void warning(const std::string& s, std::source_location location = std::source_location::current());
void info(const std::string& s, std::source_location location = std::source_location::current());

auto normalize(const float& x, const float& min, const float& max) -> float;

auto logspace(const float& start, const float& stop, const uint& npoints) -> std::vector<float>;
auto linspace(const float& start, const float& stop, const uint& npoints) -> std::vector<float>;

auto linear_to_db(const float& amp) -> float;
auto linear_to_db(const double& amp) -> double;

auto db_to_linear(const float& db) -> float;
auto db_to_linear(const double& db) -> double;

auto db_percent_to_linear(const float& percent, const float& limit = 100.0F) -> float;
auto db_percent_to_linear(const double& percent, const double& limit = 100.0) -> double;

auto db20_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data) -> gboolean;

auto linear_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant*;

auto db10_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data) -> gboolean;

auto double_to_float(GValue* value, GVariant* variant, gpointer user_data) -> gboolean;

auto db20_gain_to_linear_double(GValue* value, GVariant* variant, gpointer user_data) -> gboolean;

auto linear_double_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant*;

auto double_x10_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean;

auto ms_to_ns(GValue* value, GVariant* variant, gpointer user_data) -> gboolean;

auto remove_filename_extension(const std::string& basename) -> std::string;

void print_thread_id();

auto gchar_array_to_vector(gchar** gchar_array, const bool free_data = true) -> std::vector<std::string>;

auto make_gchar_pointer_vector(const std::vector<std::string>& input) -> std::vector<const gchar*>;

auto gsettings_get_color(GSettings* settings, const char* key) -> GdkRGBA;

auto gsettings_get_string(GSettings* settings, const char* key) -> std::string;

auto add_new_blocklist_entry(GSettings* settings, const std::string& name) -> bool;

void remove_blocklist_entry(GSettings* settings, const std::string& name);

void idle_add(std::function<void()> cb);

void generate_tags(const int& N, const std::string& start_string, const std::string& end_string);

auto get_files_name(std::filesystem::path dir_path, const std::string& ext) -> std::vector<std::string>;

void reset_all_keys_except(GSettings* settings, const std::vector<std::string>& blocklist = std::vector<std::string>());

auto str_contains(const std::string& haystack, const std::string& needle) -> bool;

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

  auto first_char = str.find_first_not_of(" +\n\r\t");

  if (first_char == std::string::npos) {
    return false;
  }

  const auto result = std::from_chars(str.data() + first_char, str.data() + str.size(), num);

  return (result.ec == std::errc());
}

template <typename T>
auto to_string(const T& num, const std::string def = "0") -> std::string {
  // This is used to replace `std::to_string` as a locale independent
  // number conversion using `std::to_chars`.
  // An additional string parameter could be eventually provided with a
  // default value to return in case the conversion fails.

  // Max buffer lenght:
  // number of base-10 digits that can be represented by the type T without change +
  // number of base-10 digits that are necessary to uniquely represent all distinct
  // values of the type T (meaningful only for real numbers) +
  // room for other characters such as "+-e,."
  const size_t max = std::numeric_limits<T>::digits10 + std::numeric_limits<T>::max_digits10 + 10u;

  std::array<char, max> buffer;

  const auto p_init = buffer.data();

  const auto result = std::to_chars(p_init, p_init + max, num);

  return (result.ec == std::errc()) ? std::string(p_init, result.ptr - p_init) : def;
}

}  // namespace util
