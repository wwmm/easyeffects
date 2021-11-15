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
#include "presets_menu_ui.hpp"

namespace ui::application_window {

G_BEGIN_DECLS

#define EE_TYPE_APPLICATION_WINDOW (application_window_get_type())

G_DECLARE_FINAL_TYPE(ApplicationWindow, application_window, EE, APP_WINDOW, AdwApplicationWindow)

G_END_DECLS

auto create(GApplication* gapp) -> ApplicationWindow*;

}  // namespace ui::application_window

#include <glibmm/i18n.h>
#include "application.hpp"
#include "pipe_info_ui.hpp"
#include "pipe_manager.hpp"

#include "stream_input_effects_ui.hpp"
#include "stream_output_effects_ui.hpp"

class ApplicationUi : public Gtk::ApplicationWindow {
 public:
  ApplicationUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Application* application);
  ApplicationUi(const ApplicationUi&) = delete;
  auto operator=(const ApplicationUi&) -> ApplicationUi& = delete;
  ApplicationUi(const ApplicationUi&&) = delete;
  auto operator=(const ApplicationUi&&) -> ApplicationUi& = delete;
  ~ApplicationUi() override;

  static auto create(Application* app) -> ApplicationUi*;

 private:
  inline static const std::string log_tag = "application_ui: ";

  Application* app;

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Stack* stack = nullptr;
  Gtk::MenuButton* presets_menu_button = nullptr;
  Gtk::ToggleButton *toggle_output = nullptr, *toggle_input = nullptr, *toggle_pipe_info = nullptr;

  std::vector<sigc::connection> connections;

  StreamOutputEffectsUi* soe_ui = nullptr;
  StreamInputEffectsUi* sie_ui = nullptr;
  PipeInfoUi* pipe_info_ui = nullptr;

  int soe_latency = 0, sie_latency = 0;
};
