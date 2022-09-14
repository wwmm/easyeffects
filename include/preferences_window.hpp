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

#pragma once

#include "preferences_general.hpp"
#include "preferences_spectrum.hpp"

namespace ui::preferences::window {

G_BEGIN_DECLS

#define EE_TYPE_PREFERENCES_WINDOW (preferences_window_get_type())

G_DECLARE_FINAL_TYPE(PreferencesWindow, preferences_window, EE, PREFERENCES_WINDOW, AdwPreferencesWindow)

G_END_DECLS

auto create() -> PreferencesWindow*;

}  // namespace ui::preferences::window