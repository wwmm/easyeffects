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

#ifndef GENERAL_SETTINGS_UI_HPP
#define GENERAL_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "application.hpp"

class GeneralSettingsUi : public Gtk::Grid {
 public:
  GeneralSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Application* application);
  GeneralSettingsUi(const GeneralSettingsUi&) = delete;
  auto operator=(const GeneralSettingsUi&) -> GeneralSettingsUi& = delete;
  GeneralSettingsUi(const GeneralSettingsUi&&) = delete;
  auto operator=(const GeneralSettingsUi&&) -> GeneralSettingsUi& = delete;
  ~GeneralSettingsUi() override;

  static void add_to_stack(Gtk::Stack* stack, Application* app);

 private:
  std::string log_tag = "general_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app = nullptr;

  Gtk::Switch *enable_autostart = nullptr, *enable_all_sinkinputs = nullptr, *enable_all_sourceoutputs = nullptr,
              *theme_switch = nullptr;

  Gtk::Button *reset_settings = nullptr, *about_button = nullptr;

  Gtk::SpinButton *realtime_priority_control = nullptr, *niceness_control = nullptr;

  Gtk::ComboBoxText* priority_type = nullptr;

  Glib::RefPtr<Gtk::Adjustment> adjustment_priority, adjustment_niceness, adjustment_audio_activity_timeout;

  std::vector<sigc::connection> connections;

  static void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& name,
                         Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  void init_autostart_switch();

  auto on_enable_autostart(bool state) -> bool;

  void on_reset_settings();

  void set_priority_controls_visibility();
};

#endif
