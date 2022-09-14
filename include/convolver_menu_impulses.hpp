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
#include <filesystem>
#include <sndfile.hh>
#include "application.hpp"
#include "ui_helpers.hpp"

namespace ui::convolver_menu_impulses {

G_BEGIN_DECLS

#define EE_TYPE_CONVOLVER_MENU_IMPULSES (convolver_menu_impulses_get_type())

G_DECLARE_FINAL_TYPE(ConvolverMenuImpulses, convolver_menu_impulses, EE, CONVOLVER_MENU_IMPULSES, GtkPopover)

G_END_DECLS

auto create() -> ConvolverMenuImpulses*;

void setup(ConvolverMenuImpulses* self, const std::string& schema_path, app::Application* application);

void append_to_string_list(ConvolverMenuImpulses* self, const std::string& irs_filename);

void remove_from_string_list(ConvolverMenuImpulses* self, const std::string& irs_filename);

}  // namespace ui::convolver_menu_impulses
