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

#include "util.hpp"

namespace util {

void debug(const std::string& s) {
  g_debug(s.c_str(), "%s");
}

void error(const std::string& s) {
  g_error(s.c_str(), "%s");
}

void critical(const std::string& s) {
  g_critical(s.c_str(), "%s");
}

void warning(const std::string& s) {
  g_warning(s.c_str(), "%s");
}

void info(const std::string& s) {
  g_info(s.c_str(), "%s");
}

void print_thread_id() {
  std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
}

auto logspace(const float& start, const float& stop, const uint& npoints) -> std::vector<float> {
  std::vector<float> output;

  if (stop <= start || npoints < 2) {
    return output;
  }

  auto log10_start = std::log10(start);
  auto log10_stop = std::log10(stop);

  const float delta = (log10_stop - log10_start) / static_cast<float>(npoints - 1);

  output.push_back(start);

  float v = log10_start;

  while (output.size() < npoints - 1) {
    v += delta;

    output.push_back(std::pow(10.0F, v));
  }

  output.push_back(stop);

  return output;
}

auto linspace(const float& start, const float& stop, const uint& npoints) -> std::vector<float> {
  std::vector<float> output;

  if (stop <= start || npoints < 2) {
    return output;
  }

  const float delta = (stop - start) / static_cast<float>(npoints - 1);

  output.push_back(start);

  float v = start;

  while (output.size() < npoints - 1) {
    v += delta;

    output.push_back(v);
  }

  output.push_back(stop);

  return output;
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
  return basename.substr(0, basename.find_last_of('.'));
}

auto timestamp_str(const time_point ts) -> std::string {
  return std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(ts.time_since_epoch()).count());
}

auto timepoint_to_long(time_point ts) -> long {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(ts.time_since_epoch()).count();
}

auto gchar_array_to_vector(gchar** gchar_array) -> std::vector<std::string> {
  std::vector<std::string> output;

  if (gchar_array != nullptr) {
    for (int n = 0; gchar_array[n] != nullptr; n++) {
      output.push_back(gchar_array[n]);
    }
  }

  g_strfreev(gchar_array);

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

  g_settings_get(settings, key, "(dddd)", &color[0], &color[1], &color[2], &color[3]);

  rgba.red = static_cast<float>(color[0]);
  rgba.green = static_cast<float>(color[1]);
  rgba.blue = static_cast<float>(color[2]);
  rgba.alpha = static_cast<float>(color[3]);

  return rgba;
}

auto add_new_blocklist_entry(GSettings* settings, const std::string& name, const char* log_tag) -> bool {
  if (name.empty()) {
    return false;
  }

  using namespace std::string_literals;

  auto list = util::gchar_array_to_vector(g_settings_get_strv(settings, "blocklist"));

  if (std::any_of(list.cbegin(), list.cend(), [&](const auto& str) { return str == name; })) {
    util::debug(log_tag + "entry already present in the list"s);

    return false;
  }

  list.push_back(name);

  g_settings_set_strv(settings, "blocklist", util::make_gchar_pointer_vector(list).data());

  util::debug(log_tag + "new entry has been added to the blocklist"s);

  return true;
}

void remove_blocklist_entry(GSettings* settings, const std::string& name, const char* log_tag) {
  using namespace std::string_literals;

  auto list = util::gchar_array_to_vector(g_settings_get_strv(settings, "blocklist"));

  list.erase(std::remove_if(list.begin(), list.end(), [=](const auto& a) { return a == name; }), list.end());

  g_settings_set_strv(settings, "blocklist", util::make_gchar_pointer_vector(list).data());

  util::debug(log_tag + "an entry has been removed from the blocklist"s);
}

void idle_add(std::function<void()> cb) {
  struct Data {
    std::function<void()> cb;
  };

  auto d = new Data();

  d->cb = cb;

  g_idle_add((GSourceFunc) +
                 [](Data* d) {
                   if (d == nullptr) {
                     return G_SOURCE_REMOVE;
                   }

                   if (d->cb == nullptr) {
                     return G_SOURCE_REMOVE;
                   }

                   d->cb();

                   delete d;

                   return G_SOURCE_REMOVE;
                 },
             d);
}

void generate_tags(const int& N, const std::string& start_string, const std::string& end_string) {
  size_t max_tag_size = 0;
  std::string body = "{";
  std::string msg = "constexpr char tag_array[][";

  for (int n = 0; n < N; n++) {
    auto n_str = std::to_string(n);

    auto tag = "\"" + start_string + n_str + end_string + "\"";

    body += "{" + tag + "}";

    if (n < N - 1) {
      body += ", ";
    }

    max_tag_size = (tag.size() > max_tag_size) ? tag.size() : max_tag_size;
  }

  msg += std::to_string(max_tag_size) + "] = " + body + "};";

  warning(msg);
}

}  // namespace util
