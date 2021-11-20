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
#include <string>
#include "application.hpp"
#include "util.hpp"

namespace ui::plugins_menu {

G_BEGIN_DECLS

#define EE_TYPE_PLUGINS_MENU (plugins_menu_get_type())

G_DECLARE_FINAL_TYPE(PluginsMenu, plugins_menu, EE, PLUGINS_MENU, GtkPopover)

G_END_DECLS

auto create() -> PluginsMenu*;

void setup(PluginsMenu* self, app::Application* application);

}  // namespace ui::plugins_menu
