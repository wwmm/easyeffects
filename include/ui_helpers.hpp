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
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include "util.hpp"

namespace ui {

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit) -> bool;

auto parse_spinbutton_input(GtkSpinButton* button, double* new_value) -> int;

template <size_t N>
struct StringLiteralWrapper {
  constexpr StringLiteralWrapper(const char (&str)[N]) : msg(std::to_array(str)) {}

  std::array<char, N> msg;
};

template <StringLiteralWrapper sl_wrapper>
void prepare_spinbutton(GtkSpinButton* button) {
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
  gtk_scale_set_format_value_func(
      scale,
      (GtkScaleFormatValueFunc) +
          [](GtkScale* scale, double value, gpointer user_data) {
            auto precision = gtk_scale_get_digits(scale);
            auto unit = sl_wrapper.msg.data();

            using namespace std::string_literals;

            auto text = fmt::format("{0:.{1}f}{2}", value, precision, ((unit != nullptr) ? " "s + unit : ""));
          },
      nullptr, nullptr);
}

void update_level(GtkLevelBar* w_left,
                  GtkLabel* w_left_label,
                  GtkLevelBar* w_right,
                  GtkLabel* w_right_label,
                  const float& left,
                  const float& right);

}  // namespace ui