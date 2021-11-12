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
#include <glib/gi18n.h>
#include <string>
#include "config.h"
#include "pipe_manager.hpp"
#include "presets_manager.hpp"
#include "stream_input_effects.hpp"
#include "stream_output_effects.hpp"

namespace app {

G_BEGIN_DECLS

#define EE_TYPE_APPLICATION (application_get_type())

G_DECLARE_FINAL_TYPE(Application, application, EE, APP, AdwApplication)

auto application_new() -> GApplication*;

void hide_all_windows(GApplication* app);

G_END_DECLS

struct _Application {
  AdwApplication parent_instance{};

  GSettings* settings = nullptr;
  GSettings* soe_settings = nullptr;
  GSettings* sie_settings = nullptr;

  std::unique_ptr<PipeManager> pm;
  std::unique_ptr<StreamOutputEffects> soe;
  std::unique_ptr<StreamInputEffects> sie;
  std::unique_ptr<PresetsManager> presets_manager;
};

}  // namespace app

class Application : public Gtk::Application {
 public:
  Application();
  Application(const Application&) = delete;
  auto operator=(const Application&) -> Application& = delete;
  Application(const Application&&) = delete;
  auto operator=(const Application&&) -> Application& = delete;
  ~Application() override;

  static auto create() -> Glib::RefPtr<Application>;

  Glib::RefPtr<Gio::Settings> settings;
  Glib::RefPtr<Gio::Settings> soe_settings;
  Glib::RefPtr<Gio::Settings> sie_settings;

  std::unique_ptr<StreamOutputEffects> soe;
  std::unique_ptr<StreamInputEffects> sie;
  std::unique_ptr<PresetsManager> presets_manager;
};
