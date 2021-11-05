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

#include <sstream>

inline auto parse_spinbutton_output(Gtk::SpinButton* b, const Glib::ustring& u) -> bool {
  b->set_text(Glib::ustring::format(std::setprecision(b->get_digits()), std::fixed, b->get_adjustment()->get_value()) +
              ((u.empty()) ? u : (" " + u)));

  return true;
}

inline auto parse_spinbutton_input(Gtk::SpinButton* button, double& new_value) -> int {
  std::istringstream str(button->get_text().c_str());

  if (auto min = 0.0, max = 0.0; str >> new_value) {
    button->get_range(min, max);

    if (new_value < min) {
      new_value = min;
    } else if (new_value > max) {
      new_value = max;
    }

    return 1;
  }

  return GTK_INPUT_ERROR;
}

#endif
