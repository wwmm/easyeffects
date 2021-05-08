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
#include "pipe_manager.hpp"

class ModuleInfoHolder : public Glib::Object {
 public:
  ModuleInfo info;

  static auto create(const ModuleInfo& info) -> Glib::RefPtr<ModuleInfoHolder>;

  sigc::signal<void(ModuleInfo)> info_updated;

 protected:
  ModuleInfoHolder(ModuleInfo info);
};

class ClientInfoHolder : public Glib::Object {
 public:
  ClientInfo info;

  static auto create(const ClientInfo& info) -> Glib::RefPtr<ClientInfoHolder>;

  sigc::signal<void(ClientInfo)> info_updated;

 protected:
  ClientInfoHolder(ClientInfo info);
};

class PipeInfoUi : public Gtk::Box {
 public:
  PipeInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PipeManager* pm_ptr);
  PipeInfoUi(const PipeInfoUi&) = delete;
  auto operator=(const PipeInfoUi&) -> PipeInfoUi& = delete;
  PipeInfoUi(const PipeInfoUi&&) = delete;
  auto operator=(const PipeInfoUi&&) -> PipeInfoUi& = delete;
  ~PipeInfoUi() override;

  static auto add_to_stack(Gtk::Stack* stack, PipeManager* pm) -> PipeInfoUi*;

 private:
  std::string log_tag = "pipe_info: ";

  PipeManager* pm = nullptr;

  Gtk::Stack* stack = nullptr;

  Gtk::Label *header_version = nullptr, *library_version = nullptr, *default_sink = nullptr, *default_source = nullptr,
             *server_rate = nullptr, *max_quantum = nullptr, *min_quantum = nullptr, *quantum = nullptr;

  Gtk::ListView *listview_modules = nullptr, *listview_clients = nullptr;

  Glib::RefPtr<ModuleInfoHolder> modules_holder;

  Glib::RefPtr<Gio::ListStore<ModuleInfoHolder>> modules_model;

  Glib::RefPtr<ClientInfoHolder> clients_holder;

  Glib::RefPtr<Gio::ListStore<ClientInfoHolder>> clients_model;

  std::vector<sigc::connection> connections;

  void setup_listview_modules();

  void setup_listview_clients();

  void update_server_info();

  void update_modules_info();

  void update_clients_info();

  void on_stack_visible_child_changed();
};

#endif
