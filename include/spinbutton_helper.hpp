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

#ifndef SPINBUTTON_HELPER_HPP
#define SPINBUTTON_HELPER_HPP

#include <gtkmm.h>
#include <sstream>

inline auto parse_spinbutton_output(Gtk::SpinButton* button, const std::string& unit) -> bool {
  std::ostringstream str;

  str.precision(button->get_digits());

  str << std::fixed << button->get_adjustment()->get_value() << " " << unit;

  button->set_text(str.str());

  return true;
}

inline auto parse_spinbutton_input(Gtk::SpinButton* button, double& new_value) {
  std::istringstream str(button->get_text());

  if (str >> new_value) {
    return 1;
  }

  return GTK_INPUT_ERROR;
}

#endif
