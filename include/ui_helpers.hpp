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
#include <sstream>
#include <string>

namespace ui {

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit) -> bool;

auto parse_spinbutton_input(GtkSpinButton* button, double* new_value) -> int;

void prepare_spinbutton(GtkSpinButton* button, const char* unit);

}  // namespace ui