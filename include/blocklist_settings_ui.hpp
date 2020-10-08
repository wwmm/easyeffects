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

#ifndef BLOCKLIST_SETTINGS_UI_HPP
#define BLOCKLIST_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "preset_type.hpp"
#include "util.hpp"

class BlocklistSettingsUi : public Gtk::Grid {
 public:
  BlocklistSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  BlocklistSettingsUi(const BlocklistSettingsUi&) = delete;
  auto operator=(const BlocklistSettingsUi&) -> BlocklistSettingsUi& = delete;
  BlocklistSettingsUi(const BlocklistSettingsUi&&) = delete;
  auto operator=(const BlocklistSettingsUi &&) -> BlocklistSettingsUi& = delete;
  ~BlocklistSettingsUi() override;

  static void add_to_stack(Gtk::Stack* stack);

  // Blocklist management static methods
  static auto add_new_entry(const std::string& name, PresetType preset_type) -> bool;
  static void remove_entry(const std::string& name, PresetType preset_type);
  static auto app_is_blocklisted(const std::string& name, PresetType preset_type) -> bool;
  static auto get_blocklisted_apps_visibility() -> bool;

 private:
  std::string log_tag = "blocklist_settings_ui: ";

  Gtk::Switch* show_blocklisted_apps = nullptr;
  Gtk::Button *add_blocklist_in = nullptr, *add_blocklist_out = nullptr;
  static Gtk::ListBox *blocklist_in_listbox, *blocklist_out_listbox;
  Gtk::Entry *blocklist_in_name = nullptr, *blocklist_out_name = nullptr;
  Gtk::ScrolledWindow *blocklist_in_scrolled_window = nullptr, *blocklist_out_scrolled_window = nullptr;

  Glib::RefPtr<Gio::Settings> settings;

  // singleton pointer used to access settings from static methods
  // needed because Gio::Settings instance can't be made static
  static BlocklistSettingsUi* thisPtr;

  static std::vector<sigc::connection> connections;

  static void populate_blocklist_in_listbox();
  static void populate_blocklist_out_listbox();
  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;
};

#endif
