#include "ui_helpers.hpp"

namespace {

bool show_user_locale_warning = true;

uint widget_serial = 0;

std::map<uint, bool> map_ignore_filter_idle_add;

}  // namespace

namespace ui {

using namespace std::string_literals;

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit) -> bool {
  auto* adjustment = gtk_spin_button_get_adjustment(button);
  auto value = gtk_adjustment_get_value(adjustment);
  auto precision = gtk_spin_button_get_digits(button);

  std::locale loc;

  try {
    loc = std::locale("");
  } catch (...) {
  }

  // format string: 0 = value, 1 = precision, 2 = unit
  auto text = fmt::format(loc, "{0:.{1}Lf}{2}", value, precision, ((unit != nullptr) ? " "s + unit : ""));

  gtk_editable_set_text(GTK_EDITABLE(button), text.c_str());

  return true;
}

auto parse_spinbutton_input(GtkSpinButton* button, double* new_value) -> int {
  std::istringstream str(gtk_editable_get_text(GTK_EDITABLE(button)));

  std::locale loc;

  try {
    loc = std::locale("");

    str.imbue(loc);
  } catch (...) {
    if (show_user_locale_warning) {
      util::warning("We could not load the user locale in your system! Your locale configuration is broken!");

      show_user_locale_warning = false;
    }

    str.imbue(loc);
  }

  if (auto min = 0.0, max = 0.0; str >> *new_value) {
    gtk_spin_button_get_range(button, &min, &max);

    *new_value = std::clamp(*new_value, min, max);

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

  if (auto db_value = util::db_to_linear(left); left >= -99.0) {
    if (db_value < 0.0) {
      db_value = 0.0;
    } else if (db_value > 1.0) {
      db_value = 1.0;
    }

    gtk_level_bar_set_value(w_left, db_value);
    gtk_label_set_text(w_left_label, fmt::format("{0:.0f}", left).c_str());
  } else {
    gtk_level_bar_set_value(w_left, 0.0);
    gtk_label_set_text(w_left_label, "-99");
  }

  if (auto db_value = util::db_to_linear(right); right >= -99.0) {
    if (db_value < 0.0) {
      db_value = 0.0;
    } else if (db_value > 1.0) {
      db_value = 1.0;
    }

    gtk_level_bar_set_value(w_right, db_value);
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
