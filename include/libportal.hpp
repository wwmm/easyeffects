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

#include <iostream>
#include "libportal-gtk4/portal-gtk4.h"
#include "libportal/portal.h"

namespace libportal {

struct Widgets {
  GtkSwitch *enable_autostart = nullptr, *shutdown_on_window_close = nullptr;
};

void on_request_background_called(GObject* source, GAsyncResult* result, Widgets* self);

void update_background_portal(const bool& state, Widgets* self);

}  // namespace libportal