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
#include <gsl/gsl_spline.h>
#include "application.hpp"
#include "apps_box.hpp"
#include "blocklist_menu.hpp"
#include "chart.hpp"
#include "effects_base.hpp"
#include "pipeline_type.hpp"
#include "plugins_box.hpp"
#include "tags_resources.hpp"

namespace ui::effects_box {

G_BEGIN_DECLS

#define EE_TYPE_EFFECTS_BOX (effects_box_get_type())

G_DECLARE_FINAL_TYPE(EffectsBox, effects_box, EE, EFFECTS_BOX, GtkBox)

G_END_DECLS

auto create() -> EffectsBox*;

void setup(EffectsBox* self, app::Application* application, PipelineType pipeline_type, GtkIconTheme* icon_theme);

}  // namespace ui::effects_box