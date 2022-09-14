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
#include <execution>
#include "convolver_ui_common.hpp"
#include "resampler.hpp"
#include "tags_resources.hpp"
#include "ui_helpers.hpp"

namespace ui::convolver_menu_combine {

G_BEGIN_DECLS

#define EE_TYPE_CONVOLVER_MENU_COMBINE (convolver_menu_combine_get_type())

G_DECLARE_FINAL_TYPE(ConvolverMenuCombine, convolver_menu_combine, EE, CONVOLVER_MENU_COMBINE, GtkPopover)

G_END_DECLS

auto create() -> ConvolverMenuCombine*;

void append_to_string_list(ConvolverMenuCombine* self, const std::string& irs_filename);

void remove_from_string_list(ConvolverMenuCombine* self, const std::string& irs_filename);

}  // namespace ui::convolver_menu_combine
