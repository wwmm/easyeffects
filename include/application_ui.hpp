/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm.h>
#include "application.hpp"
// #include "calibration_ui.hpp"
// #include "pipe_info_ui.hpp"
// #include "presets_menu_ui.hpp"
// #include "stream_input_effects_ui.hpp"
// #include "stream_output_effects_ui.hpp"

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
  std::string log_tag = "application_ui: ";

  Application* app;

  Glib::RefPtr<Gio::Settings> settings;

  std::locale global_locale;

  Gtk::Button *calibration_button = nullptr, *help_button = nullptr;
  Gtk::ToggleButton* bypass_button = nullptr;
  Gtk::Stack *stack = nullptr, *stack_menu_settings = nullptr;
  Gtk::Label* headerbar_info = nullptr;
  Gtk::Popover* presets_menu = nullptr;
  Gtk::MenuButton* presets_menu_button = nullptr;

  Gtk::Grid* subtitle_grid = nullptr;
  Gtk::HeaderBar* headerbar = nullptr;
  Gtk::Image *headerbar_icon1 = nullptr, *headerbar_icon2 = nullptr;

  std::vector<sigc::connection> connections;

  // PresetsMenuUi* presets_menu_ui = nullptr;
  // StreamOutputEffectsUi* soe_ui = nullptr;
  // StreamInputEffectsUi* sie_ui = nullptr;
  // PipeInfoUi* pipe_info_ui = nullptr;
  // CalibrationUi* calibration_ui = nullptr;

  int soe_latency = 0, sie_latency = 0;

  // static void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
  //                        const std::string& name,
  //                        Glib::RefPtr<Gtk::Adjustment>& object) {
  //   object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  // }

  void apply_css_style(const std::string& css_file_name);

  void update_headerbar_subtitle(const int& index);

  void on_stack_visible_child_changed();

  void on_calibration_button_clicked();
};

#endif
