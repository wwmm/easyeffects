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
#include "client_info_holder.hpp"
#include "module_info_holder.hpp"
#include "node_info_holder.hpp"
#include "presets_autoloading_holder.hpp"
#include "tags_resources.hpp"
#include "test_signals.hpp"
#include "ui_helpers.hpp"

namespace ui::pipe_manager_box {

G_BEGIN_DECLS

#define EE_TYPE_PIPE_MANAGER_BOX (pipe_manager_box_get_type())

G_DECLARE_FINAL_TYPE(PipeManagerBox, pipe_manager_box, EE, PIPE_MANAGER_BOX, GtkBox)

G_END_DECLS

auto create() -> PipeManagerBox*;

void setup(PipeManagerBox* self, app::Application* application);

}  // namespace ui::pipe_manager_box