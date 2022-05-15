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

#include <adwaita.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include "string_literal_wrapper.hpp"
#include "util.hpp"

namespace ui {

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit) -> bool;

auto parse_spinbutton_input(GtkSpinButton* button, double* new_value) -> int;

void update_level(GtkLevelBar* w_left,
                  GtkLabel* w_left_label,
                  GtkLevelBar* w_right,
                  GtkLabel* w_right_label,
                  const float& left,
                  const float& right);

void append_to_string_list(GtkStringList* string_list, const std::string& name);

void remove_from_string_list(GtkStringList* string_list, const std::string& name);

template <StringLiteralWrapper sl_wrapper>
void prepare_spinbutton(GtkSpinButton* button) {
  if (button == nullptr) {
    util::warning("Null pointer provided: Spinbutton widget not prepared.");

    return;
  }

  g_signal_connect(button, "output", G_CALLBACK(+[](GtkSpinButton* button, gpointer user_data) {
                     return parse_spinbutton_output(button, sl_wrapper.msg.data());
                   }),
                   nullptr);

  g_signal_connect(button, "input", G_CALLBACK(+[](GtkSpinButton* button, gdouble* new_value, gpointer user_data) {
                     return parse_spinbutton_input(button, new_value);
                   }),
                   nullptr);
}

template <StringLiteralWrapper sl_wrapper>
void prepare_scale(GtkScale* scale) {
  /*
    The sanitizer caught a "use after free" inside this function. As the problem happens randomly and is hard to
    reproduce I am not sure about what could be the cause yet. So for now I am just checking for null pointers.
  */

  if (scale == nullptr) {
    util::warning("Null pointer provided: Scale widget not prepared.");

    return;
  }

  gtk_scale_set_format_value_func(
      scale,
      (GtkScaleFormatValueFunc) +
          [](GtkScale* scale, double value, gpointer user_data) {
            if (scale == nullptr) {
              return g_strdup("");
            }

            auto precision = gtk_scale_get_digits(scale);
            auto unit = sl_wrapper.msg.data();

            using namespace std::string_literals;

            auto text = fmt::format(std::locale(setlocale(LC_ALL, nullptr)), "{0:.{1}Lf}{2}", value, precision,
                                    ((unit != nullptr) ? " "s + unit : ""));

            return g_strdup(text.c_str());
          },
      nullptr, nullptr);
}

template <StringLiteralWrapper key_wrapper, typename... Targs>
void prepare_spinbuttons(Targs... button) {
  (prepare_spinbutton<key_wrapper>(button), ...);
}

template <StringLiteralWrapper key_wrapper, typename... Targs>
void prepare_scales(Targs... scale) {
  (prepare_scale<key_wrapper>(scale), ...);
}

template <typename T>
void gsettings_bind_widget(GSettings* settings,
                           const char* key,
                           T widget,
                           GSettingsBindFlags flags = G_SETTINGS_BIND_DEFAULT) {
  static_assert(std::is_same_v<T, GtkSpinButton*> || std::is_same_v<T, GtkToggleButton*> ||
                std::is_same_v<T, GtkSwitch*> || std::is_same_v<T, GtkComboBoxText*> || std::is_same_v<T, GtkScale*>);

  if constexpr (std::is_same_v<T, GtkSpinButton*>) {
    g_settings_bind(settings, key, gtk_spin_button_get_adjustment(widget), "value", flags);
  }

  if constexpr (std::is_same_v<T, GtkScale*>) {
    g_settings_bind(settings, key, gtk_range_get_adjustment(GTK_RANGE(widget)), "value", flags);
  }

  if constexpr (std::is_same_v<T, GtkToggleButton*> || std::is_same_v<T, GtkSwitch*>) {
    g_settings_bind(settings, key, widget, "active", flags);
  }

  if constexpr (std::is_same_v<T, GtkComboBoxText*>) {
    g_settings_bind(settings, key, widget, "active-id", flags);
  }
}

template <StringLiteralWrapper... key_wrapper, typename... Targs>
void gsettings_bind_widgets(GSettings* settings, Targs... widget) {
  (gsettings_bind_widget(settings, key_wrapper.msg.data(), widget), ...);
}

}  // namespace ui
