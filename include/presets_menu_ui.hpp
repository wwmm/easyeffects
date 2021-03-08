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

#include <giomm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include "application.hpp"
#include "preset_type.hpp"

class PresetsMenuUi : public Gtk::Popover {
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

  static auto create(Application* app) -> PresetsMenuUi*;

 private:
  std::string log_tag = "presets_menu_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app = nullptr;

  Gtk::Button *add_output = nullptr, *add_input = nullptr, *import_output = nullptr, *import_input = nullptr;

  Gtk::ListView *output_listview = nullptr, *input_listview = nullptr;

  Gtk::Text *output_name = nullptr, *input_name = nullptr;

  Gtk::ScrolledWindow *output_scrolled_window = nullptr, *input_scrolled_window = nullptr;

  Gtk::SearchEntry *output_search = nullptr, *input_search = nullptr;

  Gtk::Label *last_used_output = nullptr, *last_used_input = nullptr;

  Glib::RefPtr<Gtk::StringList> output_string_list, input_string_list;

  std::vector<sigc::connection> connections;

  void create_preset(PresetType preset_type);

  void import_preset(PresetType preset_type);

  void setup_listview(Gtk::ListView* listview, PresetType preset_type, Glib::RefPtr<Gtk::StringList>& string_list);

  void reset_menu_button_label();

  auto is_autoloaded(PresetType preset_type, const std::string& name) -> bool;

  void on_show() override;
};

#endif
