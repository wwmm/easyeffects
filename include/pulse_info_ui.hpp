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

#ifndef PULSE_INFO_UI_HPP
#define PULSE_INFO_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/stack.h>
#include "pipe_manager.hpp"

class PulseInfoUi : public Gtk::Box {
 public:
  PulseInfoUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PipeManager* pm_ptr);
  PulseInfoUi(const PulseInfoUi&) = delete;
  auto operator=(const PulseInfoUi&) -> PulseInfoUi& = delete;
  PulseInfoUi(const PulseInfoUi&&) = delete;
  auto operator=(const PulseInfoUi&&) -> PulseInfoUi& = delete;
  ~PulseInfoUi() override;

  static auto add_to_stack(Gtk::Stack* stack, PipeManager* pm) -> PulseInfoUi*;

 private:
  std::string log_tag = "pulse_info: ";

  PipeManager* pm = nullptr;
  Gtk::Stack* stack = nullptr;

  Gtk::Label *server_name = nullptr, *server_version = nullptr, *default_sink = nullptr, *default_source = nullptr,
             *protocol = nullptr, *server_sample_format = nullptr, *server_rate = nullptr, *server_channels = nullptr,
             *server_channel_mapping = nullptr, *config_file = nullptr;

  Gtk::ListBox *listbox_modules = nullptr, *listbox_clients = nullptr, *listbox_config = nullptr,
               *listbox_resamplers = nullptr;

  std::vector<sigc::connection> connections;

  void update_server_info();
  void get_pulse_conf();
  void get_resamplers();

  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;
  void on_stack_visible_child_changed();
};

#endif
