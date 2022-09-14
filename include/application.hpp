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
#include <glib/gi18n.h>
#include <string>
#include "pipe_manager.hpp"
#include "preferences_window.hpp"
#include "presets_manager.hpp"
#include "stream_input_effects.hpp"
#include "stream_output_effects.hpp"

namespace app {

G_BEGIN_DECLS

#define EE_TYPE_APPLICATION (application_get_type())

G_DECLARE_FINAL_TYPE(Application, application, EE, APP, AdwApplication)

G_END_DECLS

struct Data {
 public:
  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections, gconnections_sie, gconnections_soe;
};

struct _Application {
  AdwApplication parent_instance;

  GSettings* settings;
  GSettings* soe_settings;
  GSettings* sie_settings;

  PipeManager* pm;
  StreamOutputEffects* soe;
  StreamInputEffects* sie;
  PresetsManager* presets_manager;

  Data* data;
};

auto application_new() -> GApplication*;

void hide_all_windows(GApplication* app);

}  // namespace app
