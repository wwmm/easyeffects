/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include <gdk/gdk.h>
#include <gio/gio.h>
#include <gio/gsettingsschema.h>
#include <glib-object.h>
#include <glib.h>
#include <sys/types.h>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <limits>
#include <source_location>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace util {
// Minimum dB level reported here has to be used in gsettings and spinbuttons
// as minimal values for controls that replicates the -infinity state (linear 0).
constexpr float minimum_db_level = -100.0F;
constexpr double minimum_db_d_level = -100.0;
constexpr float minimum_linear_level = 0.00001F;
constexpr double minimum_linear_d_level = 0.00001;

using source_location = std::source_location;

void debug(const std::string& s, source_location location = source_location::current());
void error(const std::string& s, source_location location = source_location::current());
void critical(const std::string& s, source_location location = source_location::current());
void warning(const std::string& s, source_location location = source_location::current());
void info(const std::string& s, source_location location = source_location::current());

auto normalize(const double& x, const double& max, const double& min = 1.0) -> double;

auto linear_to_db(const float& amp) -> float;
auto linear_to_db(const double& amp) -> double;

auto db_to_linear(const float& db) -> float;
auto db_to_linear(const double& db) -> double;

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

auto gchar_array_to_vector(gchar** gchar_array, bool free_data = true) -> std::vector<std::string>;

auto make_gchar_pointer_vector(const std::vector<std::string>& input) -> std::vector<const gchar*>;

auto gsettings_get_color(GSettings* settings, const char* key) -> GdkRGBA;

auto gsettings_get_string(GSettings* settings, const char* key) -> std::string;

auto gsettings_get_range(GSettings* settings, const char* key) -> std::pair<std::string, std::string>;

auto add_new_blocklist_entry(GSettings* settings, const std::string& name) -> bool;

void remove_blocklist_entry(GSettings* settings, const std::string& name);

void idle_add(std::function<void()> cb, std::function<void()> cleanup_cb = []() {});

auto get_files_name(const std::filesystem::path& dir_path, const std::string& ext) -> std::vector<std::string>;

void reset_all_keys_except(GSettings* settings,
                           const std::vector<std::string>& blocklist = std::vector<std::string>(),
                           bool delay = false);

auto str_contains(const std::string& haystack, const std::string& needle) -> bool;

auto compare_versions(const std::string& v0, const std::string& v1) -> int;

void str_trim_start(std::string& str);
void str_trim_end(std::string& str);
void str_trim(std::string& str);

auto search_filename(const std::filesystem::path& path,
                     const std::string& filename,
                     std::string& full_path_result,
                     const uint& top_scan_level = std::numeric_limits<uint>::max()) -> bool;

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

    /* we're asking for C locale which is preallocated, so no alloc here */

    auto loc = newlocale(LC_ALL_MASK, "C", nullptr);

    if constexpr (std::is_same_v<T, float>) {
      num = strtof_l(str.data() + first_char, &endp, loc);
    } else {
      num = strtod_l(str.data() + first_char, &endp, loc);
    }

    /* we gotta "free" it anyway */

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
  // This is used to replace `std::to_string` as a locale independent
  // number conversion using `std::to_chars`.
  // An additional string parameter could be eventually provided with a
  // default value to return in case the conversion fails.

  // Max buffer length:
  // number of base-10 digits that can be represented by the type T without change +
  // number of base-10 digits that are necessary to uniquely represent all distinct
  // values of the type T (meaningful only for real numbers) +
  // room for other characters such as "+-e,."
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

// The following is not used and it was made only for reference. May be removed in the future.
template <Number T>
auto gsettings_key_check_number_range(GSettings* settings, const char* key, const T& v) -> bool {
  GSettingsSchema* schema = nullptr;
  GVariant* g_value = nullptr;

  // Get GSettingsSchema
  g_object_get(settings, "settings-schema", &schema, nullptr);

  auto* schema_key = g_settings_schema_get_key(schema, key);

  // Get GVariant
  if constexpr (std::is_same_v<T, double>) {
    g_value = g_variant_new_double(v);
  } else if constexpr (std::is_same_v<T, int>) {
    g_value = g_variant_new_int32(v);
  }

  if (g_value == nullptr) {
    return false;
  }

  const auto is_in_range = g_settings_schema_key_range_check(schema_key, g_value);

  g_variant_unref(g_value);
  g_settings_schema_unref(schema);

  return is_in_range != 0;
}

}  // namespace util
