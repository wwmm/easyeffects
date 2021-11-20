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

#include <glib-object.h>
#include <glib.h>
#include <gtkmm.h>
#include <chrono>
#include <cmath>
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

void print_thread_id();

template <typename T>
auto glib_value(const T& cppvalue) -> Glib::Value<T> {
  Glib::Value<T> gval;

  gval.init(Glib::Value<T>::value_type());

  gval.set(cppvalue);

  return gval;
}

auto gchar_array_to_vector(gchar** gchar_array) -> std::vector<std::string>;

auto make_gchar_pointer_vector(const std::vector<std::string>& input) -> std::vector<const gchar*>;

auto gsettings_get_color(GSettings* settings, const char* key) -> GdkRGBA;

auto add_new_blocklist_entry(GSettings* settings, const std::string& name, const char* log_tag) -> bool;

void remove_blocklist_entry(GSettings* settings, const std::string& name, const char* log_tag);

}  // namespace util

#endif
