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

#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include "application.hpp"
#include "general_settings_ui.hpp"
#include "pipe_info_ui.hpp"
#include "pipe_manager.hpp"
#include "presets_menu_ui.hpp"
#include "spectrum_settings_ui.hpp"
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
  const std::string log_tag = "application_ui: ";

  Application* app;

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::ToggleButton* bypass_button = nullptr;
  Gtk::Stack *stack = nullptr, *stack_menu_settings = nullptr;
  Gtk::MenuButton* presets_menu_button = nullptr;
  Gtk::ToggleButton *toggle_output = nullptr, *toggle_input = nullptr, *toggle_pipe_info = nullptr;

  std::vector<sigc::connection> connections;

  PresetsMenuUi* presets_menu_ui = nullptr;
  StreamOutputEffectsUi* soe_ui = nullptr;
  StreamInputEffectsUi* sie_ui = nullptr;
  PipeInfoUi* pipe_info_ui = nullptr;

  int soe_latency = 0, sie_latency = 0;

  static void apply_css_style(const std::string& css_file_name);

  auto setup_icon_theme() -> Glib::RefPtr<Gtk::IconTheme>;
};

#endif
