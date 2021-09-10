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

#ifndef SCALE_HELPER_HPP
#define SCALE_HELPER_HPP

#include <gtkmm.h>

inline auto prepare_scale(Gtk::Scale* scale, const std::string& unit) {
  scale->set_format_value_func([=](const auto& value) {
    const auto& v_str =
        Glib::ustring::format(std::setprecision(scale->get_digits()), std::fixed, scale->get_adjustment()->get_value());

    return v_str + ((unit.empty()) ? "" : (" " + unit));
  });
}

#endif
