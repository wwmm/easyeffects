/*
 *  Copyright © 2017-2023 Wellington Wallace
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

namespace {

uint widget_serial = 0;

std::locale user_locale;

std::map<uint, bool> map_ignore_filter_idle_add;

}  // namespace

namespace ui {

using namespace std::string_literals;

void show_autohiding_toast(AdwToastOverlay* toast_overlay,
                           const std::string& text,
                           const uint& timeout,
                           const AdwToastPriority& priority) {
  /* ONLY AVAILABLE FROM libAdwaita 1.2
  // Construct a custom label because we want it
  // to be wrapped on reduced window width
  auto custom_title = gtk_label_new(text.c_str());

  // Set wrap properties
  gtk_label_set_wrap(GTK_LABEL(custom_title), 1);
  gtk_label_set_wrap_mode(GTK_LABEL(custom_title), PANGO_WRAP_WORD_CHAR);
  */

  // Construct AdwToast
  auto* toast = adw_toast_new("");

  // adw_toast_set_custom_title(toast, custom_title);
  adw_toast_set_title(toast, text.c_str());

  adw_toast_set_timeout(toast, timeout);

  adw_toast_set_priority(toast, priority);

  // Show AdwToast
  adw_toast_overlay_add_toast(toast_overlay, toast);
}

void show_fixed_toast(AdwToastOverlay* toast_overlay, const std::string& text, const AdwToastPriority& priority) {
  show_autohiding_toast(toast_overlay, text, 0U, priority);
}

auto missing_plugin_box(const std::string& name, const std::string& package) -> GtkWidget* {
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

    // For translators: {} is replaced by the package name.
    const auto format_descr = fmt::runtime(_("{} Is Not Installed On The System"));

    const std::string translated_name = tags::plugin_name::get_translated().at(name);

    adw_status_page_set_title(ADW_STATUS_PAGE(status_page), fmt::format(format_title, translated_name).c_str());
    adw_status_page_set_description(ADW_STATUS_PAGE(status_page), fmt::format(format_descr, package).c_str());
  } catch (...) {
  }

  adw_status_page_set_icon_name(ADW_STATUS_PAGE(status_page), "emblem-music-symbolic");

  gtk_box_append(GTK_BOX(box), status_page);

  return box;
}

void show_simple_message_dialog(GtkWidget* parent, const std::string& title, const std::string& descr) {
  if (parent == nullptr) {
    return;
  }

  // Modal flag prevents interaction with other windows in the same application
  auto* dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                        static_cast<GtkDialogFlags>(GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL),
                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, "%s", title.c_str());

  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", descr.c_str());

  // Add custom button to hint the user to press ESC to destroy the dialog.
  // This has been introduced because multiple GTK4 dialogs shown at
  // the same time could have issues on Wayland and the only way to
  // close the outer one is pressing ESC, the mouse click does not work.
  gtk_dialog_add_button(GTK_DIALOG(dialog), _("Close (Press ESC)"), 0);

  // Destroy the dialog when the user responds to it
  g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);

  // Keep the dialog on top of the main window, or center the dialog over the main window
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));

  /* Version with Adw.MessageDialog available from libAdwaita 1.2
  auto* dialog = adw_message_dialog_new(GTK_WINDOW(parent), title.c_str(), descr.c_str());

  adw_message_dialog_add_response(ADW_MESSAGE_DIALOG(dialog), "close", "OK"); */

  gtk_window_present(GTK_WINDOW(dialog));
}

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit, const bool& lower_bound) -> bool {
  auto* adjustment = gtk_spin_button_get_adjustment(button);
  auto value = gtk_adjustment_get_value(adjustment);
  auto precision = gtk_spin_button_get_digits(button);
  auto str_unit = (unit != nullptr) ? (" "s + unit) : ""s;

  bool minus_infinity = (!lower_bound && value <= util::minimum_db_d_level);

  auto text = (minus_infinity) ? (_("-inf") + str_unit)
                               // format string: 0 = value, 1 = precision, 2 = unit
                               : fmt::format(ui::get_user_locale(), "{0:.{1}Lf}{2}", value, precision, str_unit);

  gtk_editable_set_text(GTK_EDITABLE(button), text.c_str());

  return true;
}

auto parse_spinbutton_input(GtkSpinButton* button, double* new_value, const bool& lower_bound) -> int {
  auto min = 0.0;
  auto max = 0.0;

  gtk_spin_button_get_range(button, &min, &max);

  std::istringstream str(gtk_editable_get_text(GTK_EDITABLE(button)));

  if (!lower_bound) {
    auto s = str.str();

    if (s.starts_with(_("-inf"))) {
      *new_value = util::minimum_db_d_level;

      return 1;
    }
  }

  str.imbue(ui::get_user_locale());

  auto v = 0.0;

  if (str >> v) {
    *new_value = std::clamp(v, min, max);

    return 1;
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

}  // namespace ui
