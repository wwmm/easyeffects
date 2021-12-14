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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <gdk/gdk.h>
#include <glib-object.h>
#include <glib.h>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace util {

using time_point = std::chrono::time_point<std::chrono::system_clock>;

const float minimum_db_level = -100.0F;
const double minimum_db_d_level = -100.0;
const float minimum_linear_level = 0.00001F;
const double minimum_linear_d_level = 0.00001;

void debug(const std::string& s);
void error(const std::string& s);
void critical(const std::string& s);
void warning(const std::string& s);
void info(const std::string& s);

auto logspace(const float& start, const float& stop, const uint& npoints) -> std::vector<float>;
auto linspace(const float& start, const float& stop, const uint& npoints) -> std::vector<float>;

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

auto timestamp_str(time_point ts) -> std::string;

auto timepoint_to_long(time_point ts) -> long;

void print_thread_id();

auto gchar_array_to_vector(gchar** gchar_array, const bool free_data = true) -> std::vector<std::string>;

auto make_gchar_pointer_vector(const std::vector<std::string>& input) -> std::vector<const gchar*>;

auto gsettings_get_color(GSettings* settings, const char* key) -> GdkRGBA;

auto add_new_blocklist_entry(GSettings* settings, const std::string& name, const char* log_tag) -> bool;

void remove_blocklist_entry(GSettings* settings, const std::string& name, const char* log_tag);

void idle_add(std::function<void()> cb);

void generate_tags(const int& N, const std::string& start_string, const std::string& end_string);

auto get_files_name(std::filesystem::path dir_path, const std::string& ext) -> std::vector<std::string>;

void reset_all_keys(GSettings* settings);

template <typename T>
void print_type(T v) {
  warning(typeid(v).name());
}

}  // namespace util

#endif
