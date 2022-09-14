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

#include <adwaita.h>
#include "application.hpp"
#include "preset_type.hpp"
#include "tags_resources.hpp"

namespace ui::presets_menu {

G_BEGIN_DECLS

#define EE_TYPE_PRESETS_MENU (presets_menu_get_type())

G_DECLARE_FINAL_TYPE(PresetsMenu, presets_menu, EE, PRESETS_MENU, GtkPopover)

G_END_DECLS

auto create() -> PresetsMenu*;

void setup(PresetsMenu* self, app::Application* application);

}  // namespace ui::presets_menu
