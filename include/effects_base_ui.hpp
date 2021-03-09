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

#ifndef EFFECTS_BASE_UI_HPP
#define EFFECTS_BASE_UI_HPP

#include <giomm.h>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>
#include <vector>
#include "pipe_manager.hpp"
#include "preset_type.hpp"
#include "spectrum_ui.hpp"

class NodeInfoHolder : public Glib::Object {
 public:
  NodeInfo info;

  static auto create(const NodeInfo& info) -> Glib::RefPtr<NodeInfoHolder>;

  sigc::signal<void(NodeInfo)> info_updated;

 protected:
  NodeInfoHolder(NodeInfo info);
};

class EffectsBaseUi {
 public:
  EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                Glib::RefPtr<Gio::Settings> refSettings,
                PipeManager* pipe_manager);
  EffectsBaseUi(const EffectsBaseUi&) = delete;
  auto operator=(const EffectsBaseUi&) -> EffectsBaseUi& = delete;
  EffectsBaseUi(const EffectsBaseUi&&) = delete;
  auto operator=(const EffectsBaseUi&&) -> EffectsBaseUi& = delete;
  virtual ~EffectsBaseUi();

 protected:
  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Image *app_input_icon = nullptr, *app_output_icon = nullptr, *saturation_icon = nullptr;
  Gtk::Label *global_output_level_left = nullptr, *global_output_level_right = nullptr, *device_state = nullptr;

  PipeManager* pm = nullptr;

  std::vector<sigc::connection> connections;

  /*
    Do not pass node_info by reference. Sometimes it dies before we use it and a segmentation fault happens
  */

  void on_app_added(NodeInfo node_info);
  void on_app_changed(NodeInfo node_info);
  void on_app_removed(NodeInfo node_info);

  void on_new_output_level_db(const std::array<double, 2>& peak);

  static auto node_state_to_string(const pw_node_state& state) -> std::string;

 private:
  Gtk::Stack* stack_top = nullptr;
  Gtk::ListView *listview_players = nullptr, *listview_blocklist = nullptr, *listview_selected_plugins = nullptr,
                *listview_menu_plugins = nullptr;
  Gtk::Switch* show_blocklisted_apps = nullptr;
  Gtk::Button* button_add_to_blocklist = nullptr;
  Gtk::Text* blocklist_player_name = nullptr;
  Gtk::ScrolledWindow *blocklist_scrolled_window = nullptr, *scrolled_window_menu_plugins = nullptr;
  Gtk::MenuButton *menubutton_blocklist = nullptr, *menubutton_select_plugin = nullptr;
  Gtk::Popover *popover_blocklist = nullptr, *popover_select_plugin = nullptr;

  Glib::RefPtr<NodeInfoHolder> players_holder;

  Glib::RefPtr<Gio::ListStore<NodeInfoHolder>> players_model, all_players_model;

  Glib::RefPtr<Gtk::StringList> blocklist;

  template <typename T>
  auto level_to_localized_string_showpos(const T& value, const int& places) -> std::string {
    std::ostringstream msg;

    msg.precision(places);

    msg << ((value > 0.0) ? "+" : "") << std::fixed << value;

    return msg.str();
  }

  void setup_listview_players();

  void setup_listview_blocklist();

  auto app_is_blocklisted(const Glib::ustring& name) -> bool;

  auto add_new_blocklist_entry(const Glib::ustring& name) -> bool;

  void remove_blocklist_entry(const Glib::ustring& name);

  auto float_to_localized_string(const float& value, const int& places) -> std::string;
};

#endif
