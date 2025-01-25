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

#include "ui_helpers.hpp"
#include <adwaita.h>
#include <glib-object.h>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtk/gtkshortcut.h>
#include <sys/types.h>
#include <algorithm>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

namespace {

uint widget_serial = 0U;

std::locale user_locale;

std::map<uint, bool> map_ignore_filter_idle_add;

GSettings* global_app_settings = nullptr;

}  // namespace

namespace ui {

using namespace std::string_literals;

void show_autohiding_toast(AdwToastOverlay* toast_overlay,
                           const std::string& text,
                           const uint& timeout,
                           const AdwToastPriority& priority) {
  // Construct AdwToast
  auto* toast = adw_toast_new(text.c_str());

  adw_toast_set_timeout(toast, timeout);
  adw_toast_set_priority(toast, priority);

  // Show AdwToast
  adw_toast_overlay_add_toast(toast_overlay, toast);
}

void show_fixed_toast(AdwToastOverlay* toast_overlay, const std::string& text, const AdwToastPriority& priority) {
  show_autohiding_toast(toast_overlay, text, 0U, priority);
}

auto missing_plugin_box(const std::string& base_name, const std::string& package) -> GtkWidget* {
  // Since the plugin name should be translated in the local language,
  // this function needs the base name as parameter, retrieved from
  // get_base_name() util.

  auto* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

  gtk_widget_set_margin_start(box, 6);
  gtk_widget_set_margin_end(box, 6);
  gtk_widget_set_margin_bottom(box, 6);
  gtk_widget_set_margin_top(box, 6);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  auto* status_page = adw_status_page_new();

  try {
    // For translators: {} is replaced by the effect name.
    const auto format_title = fmt::runtime(_("{} Not Available"));

    // For translators: the first {} is replaced by the effect name, the second {} is replaced by the package name.
    auto translated_descr =
        _("The software required for the {} effect, \"{}\", is not installed. Consider using the Easy Effects Flatpak "
          "package or installing the software yourself. If you've installed the missing software, you must restart "
          "Easy Effects using the 'Quit' option inside the menu in order to make the effect availble to use.");

    if (base_name == tags::plugin_name::rnnoise) {
      // For translators: the first {} is replaced by the effect name, the second {} is replaced by the package name.
      translated_descr =
          _("The {} effect was disabled when Easy Effects was compiled. This is perhaps since the "
            "software required for this effect, \"{}\", was not available. Consider using the Easy "
            "Effects Flatpak package or building your own Easy Effects package.");
    }

    const auto format_descr = fmt::runtime(translated_descr);

    const std::string translated_name = tags::plugin_name::get_translated().at(base_name);

    adw_status_page_set_title(ADW_STATUS_PAGE(status_page), fmt::format(format_title, translated_name).c_str());
    adw_status_page_set_description(ADW_STATUS_PAGE(status_page),
                                    fmt::format(format_descr, translated_name, package).c_str());
  } catch (std::out_of_range& e) {
    util::debug(e.what());
  }

  adw_status_page_set_icon_name(ADW_STATUS_PAGE(status_page), "emblem-music-symbolic");

  gtk_box_append(GTK_BOX(box), status_page);

  return box;
}

void show_simple_message_dialog(GtkWidget* parent, const std::string& title, const std::string& descr) {
  if (parent == nullptr) {
    return;
  }

  auto* dialog = adw_alert_dialog_new(title.c_str(), descr.c_str());

  const std::string response_id = "close";
  adw_alert_dialog_add_response(ADW_ALERT_DIALOG(dialog), response_id.c_str(), "Close (Press ESC)");
  adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), response_id.c_str());

  adw_dialog_present(dialog, parent);
}

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit, const bool& lower_bound) -> gboolean {
  auto* adjustment = gtk_spin_button_get_adjustment(button);
  const auto value = gtk_adjustment_get_value(adjustment);
  const auto precision = gtk_spin_button_get_digits(button);
  const auto str_unit = (unit != nullptr) ? (" "s + unit) : "";

  const bool minus_infinity = (!lower_bound && value <= util::minimum_db_d_level);

  auto text = (minus_infinity) ? (_("-inf") + str_unit)
                               // format string: 0 = value, 1 = precision, 2 = unit
                               : fmt::format(ui::get_user_locale(), "{0:.{1}Lf}{2}", value, precision, str_unit);

  gtk_editable_set_text(GTK_EDITABLE(button), text.c_str());

  return TRUE;
}

auto parse_spinbutton_input(GtkSpinButton* button, gdouble* new_value, const bool& lower_bound) -> gint {
  auto min = 0.0;
  auto max = 0.0;

  gtk_spin_button_get_range(button, &min, &max);

  std::string s = gtk_editable_get_text(GTK_EDITABLE(button));

  // Workaround to fix locale issue in #3532; see PR #3557.
  s.erase(std::remove(s.begin(), s.end(), ' '), s.end());

  if (!lower_bound && s.starts_with(_("-inf"))) {
    *new_value = util::minimum_db_d_level;

    return TRUE;
  }

  std::istringstream str(s);
  str.imbue(ui::get_user_locale());

  auto v = 0.0;

  if (str >> v) {
    *new_value = std::clamp(v, min, max);

    return TRUE;
  }

  return GTK_INPUT_ERROR;
}

auto get_new_filter_serial() -> uint {
  widget_serial++;

  return widget_serial;
}

void set_ignore_filter_idle_add(const uint& serial, const bool& state) {
  map_ignore_filter_idle_add[serial] = state;
}

auto get_ignore_filter_idle_add(const uint& serial) -> bool {
  return map_ignore_filter_idle_add[serial];
}

void save_user_locale() {
  try {
    user_locale = std::locale("");
  } catch (...) {
    util::warning("We could not get the user locale in your system! Your locale configuration is broken!");

    util::warning("Falling back to the C locale");
  }
}

auto get_user_locale() -> std::locale {
  return user_locale;
}

void update_level(GtkLevelBar* w_left,
                  GtkLabel* w_left_label,
                  GtkLevelBar* w_right,
                  GtkLabel* w_right_label,
                  const float& left,
                  const float& right) {
  if (!GTK_IS_LEVEL_BAR(w_left) || !GTK_IS_LABEL(w_left_label) || !GTK_IS_LEVEL_BAR(w_right) ||
      !GTK_IS_LABEL(w_right_label)) {
    return;
  }

  if (left >= -99.0F) {
    // Level bar widget needs double value
    const auto linear_value = static_cast<double>(std::clamp(util::db_to_linear(left), 0.0F, 1.0F));

    gtk_level_bar_set_value(w_left, linear_value);
    gtk_label_set_text(w_left_label, fmt::format("{0:.0f}", left).c_str());
  } else {
    gtk_level_bar_set_value(w_left, 0.0);
    gtk_label_set_text(w_left_label, "-99");
  }

  if (right >= -99.0F) {
    // Level bar widget needs double value
    const auto linear_value = static_cast<double>(std::clamp(util::db_to_linear(right), 0.0F, 1.0F));

    gtk_level_bar_set_value(w_right, linear_value);
    gtk_label_set_text(w_right_label, fmt::format("{0:.0f}", right).c_str());
  } else {
    gtk_level_bar_set_value(w_right, 0.0);
    gtk_label_set_text(w_right_label, "-99");
  }
}

auto get_plugin_credit_translated(const std::string& plugin_package) -> std::string {
  try {
    // For translators: {} is replaced by the library used by the plugin. I.e. "Using Calf Studio".
    const auto format = fmt::runtime(_("Using {}"));

    return fmt::format(format, plugin_package);
  } catch (...) {
    return plugin_package;
  }
}

void append_to_string_list(GtkStringList* string_list, const std::string& name) {
  for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(string_list)); n++) {
    if (gtk_string_list_get_string(string_list, n) == name) {
      return;
    }
  }

  gtk_string_list_append(string_list, name.c_str());
}

void remove_from_string_list(GtkStringList* string_list, const std::string& name) {
  for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(string_list)); n++) {
    if (gtk_string_list_get_string(string_list, n) == name) {
      gtk_string_list_remove(string_list, n);

      return;
    }
  }
}

void init_global_app_settings() {
  global_app_settings = g_settings_new(tags::app::id);
}

void unref_global_app_settings() {
  g_object_unref(global_app_settings);
}

auto get_global_app_settings() -> GSettings* {
  return global_app_settings;
}

}  // namespace ui
