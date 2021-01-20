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

#ifndef PRESETS_MENU_UI_HPP
#define PRESETS_MENU_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/popover.h>
#include <gtkmm/scrolledwindow.h>
#include "application.hpp"
#include "preset_type.hpp"

class PresetsMenuUi : public Gtk::Grid {
 public:
  PresetsMenuUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                Glib::RefPtr<Gio::Settings> refSettings,
                Application* application);
  PresetsMenuUi(const PresetsMenuUi&) = delete;
  auto operator=(const PresetsMenuUi&) -> PresetsMenuUi& = delete;
  PresetsMenuUi(const PresetsMenuUi&&) = delete;
  auto operator=(const PresetsMenuUi&&) -> PresetsMenuUi& = delete;
  ~PresetsMenuUi() override;

  static auto add_to_popover(Gtk::Popover* popover, Application* app) -> PresetsMenuUi*;

  void on_presets_menu_button_clicked();

 private:
  std::string log_tag = "presets_menu_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app = nullptr;

  Gtk::Button *add_output = nullptr, *add_input = nullptr, *import_output = nullptr, *import_input = nullptr;
  Gtk::ListBox *output_listbox = nullptr, *input_listbox = nullptr;

  Gtk::Entry *output_name = nullptr, *input_name = nullptr;
  Gtk::ScrolledWindow *output_scrolled_window = nullptr, *input_scrolled_window = nullptr;

  std::vector<sigc::connection> connections;

  void create_preset(PresetType preset_type);

  void import_preset(PresetType preset_type);

  void populate_listbox(PresetType preset_type);

  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;

  void reset_menu_button_label();

  auto is_autoloaded(PresetType preset_type, const std::string& name) -> bool;
};

#endif
