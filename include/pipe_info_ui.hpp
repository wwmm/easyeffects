/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#ifndef PIPE_INFO_UI_HPP
#define PIPE_INFO_UI_HPP

#include <giomm.h>
#include <gtkmm.h>
#include <filesystem>
#include <fstream>
#include "info_holders.hpp"
#include "pipe_manager.hpp"
#include "presets_manager.hpp"

class PipeInfoUi : public Gtk::Box {
 public:
  PipeInfoUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder,
             PipeManager* pm_ptr,
             PresetsManager* presets_manager);
  PipeInfoUi(const PipeInfoUi&) = delete;
  auto operator=(const PipeInfoUi&) -> PipeInfoUi& = delete;
  PipeInfoUi(const PipeInfoUi&&) = delete;
  auto operator=(const PipeInfoUi&&) -> PipeInfoUi& = delete;
  ~PipeInfoUi() override;

  static auto add_to_stack(Gtk::Stack* stack, PipeManager* pm, PresetsManager* presets_manager) -> PipeInfoUi*;

 private:
  std::string log_tag = "pipe_info: ";

  PipeManager* pm = nullptr;

  PresetsManager* presets_manager = nullptr;

  Gtk::Stack* stack = nullptr;

  Gtk::Switch* use_default_input = nullptr;

  Gtk::Switch* use_default_output = nullptr;

  Gtk::Label *header_version = nullptr, *library_version = nullptr, *server_rate = nullptr, *max_quantum = nullptr,
             *min_quantum = nullptr, *quantum = nullptr;

  Gtk::DropDown* dropdown_input_devices = nullptr;

  Gtk::DropDown* dropdown_output_devices = nullptr;

  Gtk::DropDown* dropdown_autoloading_output_devices = nullptr;

  Gtk::DropDown* dropdown_autoloading_input_devices = nullptr;

  Gtk::DropDown* dropdown_autoloading_output_presets = nullptr;

  Gtk::DropDown* dropdown_autoloading_input_presets = nullptr;

  Gtk::ListView* listview_modules = nullptr;

  Gtk::ListView* listview_clients = nullptr;

  Gtk::ListView* listview_autoloading_output = nullptr;

  Gtk::ListView* listview_autoloading_input = nullptr;

  Glib::RefPtr<Gio::Settings> sie_settings;

  Glib::RefPtr<Gio::Settings> soe_settings;

  Glib::RefPtr<NodeInfoHolder> input_devices_holder, output_devices_holder;

  Glib::RefPtr<ModuleInfoHolder> modules_holder;

  Glib::RefPtr<ClientInfoHolder> clients_holder;

  Glib::RefPtr<PresetsAutoloadingHolder> autoloading_output_holder;

  Glib::RefPtr<PresetsAutoloadingHolder> autoloading_input_holder;

  Glib::RefPtr<Gio::ListStore<NodeInfoHolder>> input_devices_model, output_devices_model;

  Glib::RefPtr<Gio::ListStore<ModuleInfoHolder>> modules_model;

  Glib::RefPtr<Gio::ListStore<ClientInfoHolder>> clients_model;

  Glib::RefPtr<Gio::ListStore<PresetsAutoloadingHolder>> autoloading_output_model;

  Glib::RefPtr<Gio::ListStore<PresetsAutoloadingHolder>> autoloading_input_model;

  Glib::RefPtr<Gtk::StringList> output_presets_string_list;

  Glib::RefPtr<Gtk::StringList> input_presets_string_list;

  std::vector<sigc::connection> connections;

  static void setup_dropdown_devices(Gtk::DropDown* dropdown,
                                     const Glib::RefPtr<Gio::ListStore<NodeInfoHolder>>& model);

  void setup_dropdown_presets(PresetType preset_type, const Glib::RefPtr<Gtk::StringList>& string_list);

  void setup_listview_modules();

  void setup_listview_clients();

  void setup_listview_autoloading(Gtk::ListView* listview,
                                  const Glib::RefPtr<Gio::ListStore<PresetsAutoloadingHolder>>& model);

  void update_modules_info();

  void update_clients_info();

  void on_stack_visible_child_changed();
};

#endif
