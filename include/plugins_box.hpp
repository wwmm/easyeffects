/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include <glib-object.h>
#include <glibconfig.h>
#include <gtk/gtk.h>
#include "application.hpp"
#include "pipeline_type.hpp"

namespace ui::plugins_box {

G_BEGIN_DECLS

#define EE_TYPE_PLUGINS_BOX (plugins_box_get_type())

G_DECLARE_FINAL_TYPE(PluginsBox, plugins_box, EE, PLUGINS_BOX, GtkBox)

G_END_DECLS

auto create() -> PluginsBox*;

void setup(PluginsBox* self, app::Application* application, PipelineType pipeline_type);

}  // namespace ui::plugins_box
