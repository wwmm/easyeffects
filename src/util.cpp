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

#include "util.hpp"
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <gio/gsettingsschema.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gvariant.h>
#include <glib/gvarianttype.h>
#include <glibconfig.h>
#include <sys/types.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "pipe_manager.hpp"

namespace util {

auto prepare_debug_message(const std::string& message, source_location location) -> std::string {
  auto file_path = std::filesystem::path{location.file_name()};

  std::ostringstream msg_stream;

  msg_stream << "\t" << file_path.filename().string() << ":" << to_string(location.line()) << "\t" << message;

  return msg_stream.str();
}

void debug(const std::string& s, source_location location) {
  g_debug(prepare_debug_message(s, location).c_str(), "%s");
}

void error(const std::string& s, source_location location) {
  g_error(prepare_debug_message(s, location).c_str(), "%s");
}

void critical(const std::string& s, source_location location) {
  g_critical(prepare_debug_message(s, location).c_str(), "%s");
}

void warning(const std::string& s, source_location location) {
  g_warning(prepare_debug_message(s, location).c_str(), "%s");
}

void info(const std::string& s, source_location location) {
  g_info(prepare_debug_message(s, location).c_str(), "%s");
}

void print_thread_id() {
  std::cout << "thread id: " << std::this_thread::get_id() << '\n';
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
  return std::exp((db / 20.0F) * std::log(10.0F));
}

auto db_to_linear(const double& db) -> double {
  return std::exp((db / 20.0) * std::log(10.0));
}

auto db20_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const gfloat v_linear = std::pow(10.0F, static_cast<float>(g_variant_get_double(variant)) / 20.0F);

  g_value_set_float(value, v_linear);

  return 1;
}

auto linear_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data) -> GVariant* {
  const gdouble v_db = 20.0 * std::log10(static_cast<double>(g_value_get_float(value)));

  return g_variant_new_double(v_db);
}

auto db10_gain_to_linear(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const gfloat v_linear = std::pow(10.0F, static_cast<float>(g_variant_get_double(variant)) / 10.0F);

  g_value_set_float(value, v_linear);

  return 1;
}

auto double_to_float(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  g_value_set_float(value, static_cast<gfloat>(g_variant_get_double(variant)));

  return 1;
}

auto db20_gain_to_linear_double(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  const gdouble v_linear = std::pow(10.0, g_variant_get_double(variant) / 20.0);

  g_value_set_double(value, v_linear);

  return 1;
}

auto linear_double_gain_to_db20(const GValue* value, const GVariantType* expected_type, gpointer user_data)
    -> GVariant* {
  const gdouble v_db = 20.0 * std::log10(g_value_get_double(value));

  return g_variant_new_double(v_db);
}

auto double_x10_to_int(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  g_value_set_int(value, static_cast<gint>(g_variant_get_double(variant) * 10.0));

  return 1;
}

auto ms_to_ns(GValue* value, GVariant* variant, gpointer user_data) -> gboolean {
  g_value_set_uint64(value, static_cast<guint64>(g_variant_get_double(variant) * 1000000.0));

  return 1;
}

auto remove_filename_extension(const std::string& basename) -> std::string {
  return basename.substr(0U, basename.find_last_of('.'));
}

auto gchar_array_to_vector(gchar** gchar_array, const bool free_data) -> std::vector<std::string> {
  std::vector<std::string> output;

  if (gchar_array != nullptr) {
    for (int n = 0; gchar_array[n] != nullptr; n++) {
      output.emplace_back(gchar_array[n]);
    }
  }

  if (free_data) {
    g_strfreev(gchar_array);
  }

  return output;
}

auto make_gchar_pointer_vector(const std::vector<std::string>& input) -> std::vector<const gchar*> {
  std::vector<const gchar*> output;

  output.reserve(input.size());

  for (const auto& v : input) {
    output.push_back(v.c_str());
  }

  output.push_back(nullptr);  // char* arrays passed to g_settings_set_strv must have a null pointer as the last element

  return output;
}

auto gsettings_get_color(GSettings* settings, const char* key) -> GdkRGBA {
  GdkRGBA rgba;
  std::array<double, 4> color{};

  g_settings_get(settings, key, "(dddd)", color.data(), &color[1], &color[2], &color[3]);

  rgba.red = static_cast<float>(color[0]);
  rgba.green = static_cast<float>(color[1]);
  rgba.blue = static_cast<float>(color[2]);
  rgba.alpha = static_cast<float>(color[3]);

  return rgba;
}

auto gsettings_get_string(GSettings* settings, const char* key) -> std::string {
  auto* s = g_settings_get_string(settings, key);

  std::string output = s;

  g_free(s);

  return output;
}

// The following is not used and it was made only for reference. May be removed in the future.
// GIO recommends to not use g_settings_schema_key_get_range in "normal programs".
auto gsettings_get_range(GSettings* settings, const char* key) -> std::pair<std::string, std::string> {
  GSettingsSchema* schema = nullptr;
  const gchar* type = nullptr;
  GVariant* detail = nullptr;
  std::string min_v;
  std::string max_v;

  g_object_get(settings, "settings-schema", &schema, nullptr);

  auto* schema_key = g_settings_schema_get_key(schema, key);

  auto* range = g_settings_schema_key_get_range(schema_key);

  g_variant_get(range, "(&sv)", &type, &detail);

  if (strcmp(type, "range") == 0) {
    GVariant* min = nullptr;
    GVariant* max = nullptr;
    gchar* smin = nullptr;
    gchar* smax = nullptr;

    g_variant_get(detail, "(**)", &min, &max);

    smin = g_variant_print(min, 0);
    smax = g_variant_print(max, 0);

    min_v = smin;
    max_v = smax;

    g_variant_unref(min);
    g_variant_unref(max);
    g_free(smin);
    g_free(smax);
  }

  g_variant_unref(detail);
  g_variant_unref(range);
  g_settings_schema_key_unref(schema_key);
  g_settings_schema_unref(schema);

  return {min_v, max_v};
}

auto add_new_blocklist_entry(GSettings* settings, const std::string& name) -> bool {
  if (name.empty()) {
    return false;
  }

  auto list = util::gchar_array_to_vector(g_settings_get_strv(settings, "blocklist"));

  if (std::any_of(list.cbegin(), list.cend(), [&](const auto& str) { return str == name; })) {
    util::debug("entry already present in the list");

    return false;
  }

  list.push_back(name);

  g_settings_set_strv(settings, "blocklist", util::make_gchar_pointer_vector(list).data());

  util::debug("new entry has been added to the blocklist");

  return true;
}

void remove_blocklist_entry(GSettings* settings, const std::string& name) {
  auto list = util::gchar_array_to_vector(g_settings_get_strv(settings, "blocklist"));

  list.erase(std::remove_if(list.begin(), list.end(), [=](const auto& a) { return a == name; }), list.end());

  g_settings_set_strv(settings, "blocklist", util::make_gchar_pointer_vector(list).data());

  util::debug("an entry has been removed from the blocklist");
}

void idle_add(std::function<void()> cb, std::function<void()> cleanup_cb) {
  struct Data {
    std::function<void()> cb, cleanup_cp;
  };

  auto* d = new Data();

  d->cb = std::move(cb);
  d->cleanup_cp = std::move(cleanup_cb);

  g_idle_add((GSourceFunc) +
                 [](Data* d) {
                   if (d == nullptr) {
                     return G_SOURCE_REMOVE;
                   }

                   if (PipeManager::exiting) {
                     delete d;
                     return G_SOURCE_REMOVE;
                   }

                   if (d->cb == nullptr) {
                     return G_SOURCE_REMOVE;
                   }

                   d->cb();

                   d->cleanup_cp();

                   delete d;

                   return G_SOURCE_REMOVE;
                 },
             d);
}

auto get_files_name(const std::filesystem::path& dir_path, const std::string& ext) -> std::vector<std::string> {
  std::vector<std::string> names;

  for (std::filesystem::directory_iterator it{dir_path}; it != std::filesystem::directory_iterator{}; ++it) {
    if (std::filesystem::is_regular_file(it->status())) {
      if (it->path().extension() == ext) {
        names.push_back(it->path().stem().string());
      }
    }
  }

  return names;
}

void reset_all_keys_except(GSettings* settings, const std::vector<std::string>& blocklist, bool delay) {
  GSettingsSchema* schema = nullptr;

  g_object_get(settings, "settings-schema", &schema, nullptr);

  gchar** keys = g_settings_schema_list_keys(schema);

  /*
    For some reason the delayed mode breaks the binding between gsettings and the plugins. Until We figure out the
    reasons delayed mode will be disabled.
  */

  // Gsettings should have a maximum of 256 delayed changes in delay mode (see issue #2215).
  // If surpassed, the whole application crashes (it happens on the Equalizer).
  // Anyway we set the maximum at the half (128) for satefy reasons.
  uint keys_changed = 0U;
  uint max_changes = 128U;

  if (delay) {
    g_settings_delay(settings);
  }

  for (int i = 0; keys[i] != nullptr; i++) {
    if (std::ranges::find(blocklist, keys[i]) != blocklist.end()) {
      continue;
    }

    g_settings_reset(settings, keys[i]);
    keys_changed++;

    if (delay && keys_changed >= max_changes) {
      g_settings_apply(settings);
      keys_changed = 0U;
    }
  }

  if (delay && keys_changed > 0U) {
    g_settings_apply(settings);
  }

  g_settings_schema_unref(schema);
  g_strfreev(keys);
}

auto str_contains(const std::string& haystack, const std::string& needle) -> bool {
  // This helper indicates if the needle is contained in the haystack string,
  // but the empty needle will NOT return true.

  // Instead .find method of C++ string class returns a size_type different
  // than std::string::npos when the needle is empty indicating that an empty
  // string IS CONTAINED in the haystack. That's pointless, so here is this helper.

  if (needle.empty()) {
    return false;
  }

  return (haystack.find(needle) != std::string::npos);
}

void str_trim_start(std::string& str) {
  // This util removes whitespaces such as simple space " ", new line "\n",
  // carriage return "\r", tab "\t", vertical tab "\v" and form feed "\f"
  // at the start of the given string.
  // No copy involved, the input string is just modified if needed.

  str.erase(0U, str.find_first_not_of(" \n\r\t\v\f"));
}

void str_trim_end(std::string& str) {
  // Same as above, but at the end of the given string.
  str.erase(str.find_last_not_of(" \n\r\t\v\f") + 1U);
}
void str_trim(std::string& str) {
  // Trim both sides of the given string. See above.
  str_trim_end(str);
  str_trim_start(str);
}

auto search_filename(const std::filesystem::path& path,
                     const std::string& filename,
                     std::string& full_path_result,
                     const uint& top_scan_level) -> bool {
  // Recursive util to search a filename from an origin full path directory.
  // The search is performed in subdirectories and it's stopped at a specified
  // sublevel (top_scan_level = 1 searches only in the path).

  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    return false;
  }

  const auto fn = path / filename;

  // Get the file in this directory, if exists.
  if (std::filesystem::exists(fn) && std::filesystem::is_regular_file(fn)) {
    // File found, abort the search.
    full_path_result = fn.c_str();

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
  /* This is an util to compare two strings as semver, mainly used to compare
     two Pipewire versions.
     The format should adhere to what is defined at `https://semver.org/`.
     The additional extension label, if present, is ignored and fortunately
     we don't need to look at it since Pipewire does not use it.

     Given two version strings v0 and v1, this util returns an integer:
     - 0 if the versions are equal;
     - 1 if v0 is higher than v1;
     - -1 if v0 is lower than v1;
     - Whichever other number if the comparison fails (i.e. giving one or
       both strings not respecting the semver format).
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

}  // namespace util
