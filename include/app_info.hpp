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
#include <fmt/core.h>
#include "application.hpp"
#include "node_info_holder.hpp"
#include "tags_resources.hpp"

namespace ui::app_info {

G_BEGIN_DECLS

#define EE_TYPE_APP_INFO (app_info_get_type())

G_DECLARE_FINAL_TYPE(AppInfo, app_info, EE, APP_INFO, GtkBox)

G_END_DECLS

auto create() -> AppInfo*;

void setup(AppInfo* self,
           app::Application* application,
           GSettings* settings,
           GtkIconTheme* icon_theme,
           std::unordered_map<uint, bool>& enabled_app_list);

void update(AppInfo* self, NodeInfo node_info);

}  // namespace ui::app_info
