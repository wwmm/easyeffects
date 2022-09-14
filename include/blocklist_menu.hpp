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
#include <string>
#include "application.hpp"
#include "pipeline_type.hpp"
#include "tags_resources.hpp"

namespace ui::blocklist_menu {

G_BEGIN_DECLS

#define EE_TYPE_BLOCKLIST_MENU (blocklist_menu_get_type())

G_DECLARE_FINAL_TYPE(BlocklistMenu, blocklist_menu, EE, BLOCKLIST_MENU, GtkPopover)

G_END_DECLS

auto create() -> BlocklistMenu*;

void setup(BlocklistMenu* self, app::Application* application, PipelineType pipeline_type);

}  // namespace ui::blocklist_menu
