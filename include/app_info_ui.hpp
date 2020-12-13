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

#ifndef APP_INFO_UI_HPP
#define APP_INFO_UI_HPP

#include <glibmm/i18n.h>
#include <gtkmm/builder.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/switch.h>
#include <gtkmm/togglebutton.h>
#include "blocklist_settings_ui.hpp"
#include "pipe_manager.hpp"
#include "preset_type.hpp"
#include "util.hpp"

class AppInfoUi : public Gtk::Grid {
 public:
  AppInfoUi(BaseObjectType* cobject,
            const Glib::RefPtr<Gtk::Builder>& builder,
            NodeInfo node_info,
            PipeManager* pulse_manager);
  AppInfoUi(const AppInfoUi&) = delete;
  auto operator=(const AppInfoUi&) -> AppInfoUi& = delete;
  AppInfoUi(const AppInfoUi&&) = delete;
  auto operator=(const AppInfoUi&&) -> AppInfoUi& = delete;
  ~AppInfoUi() override;

  Gtk::Switch* enable = nullptr;

  Gtk::ToggleButton* mute = nullptr;
  Gtk::CheckButton* blocklist = nullptr;

  Gtk::Scale* volume = nullptr;

  Gtk::Image* app_icon = nullptr;
  Gtk::Image* mute_icon = nullptr;

  Gtk::Label* app_name = nullptr;
  Gtk::Label* media_name = nullptr;
  Gtk::Label* format = nullptr;
  Gtk::Label* rate = nullptr;
  Gtk::Label* channels = nullptr;
  Gtk::Label* resampler = nullptr;
  Gtk::Label* buffer = nullptr;
  Gtk::Label* latency = nullptr;
  Gtk::Label* state = nullptr;

  NodeInfo nd_info;

  void update(NodeInfo node_info);

 private:
  std::string log_tag = "app_info_ui: ";

  bool running = true, is_enabled = true, is_blocklisted = true, pre_bl_state = true;

  std::locale global_locale;

  sigc::connection enable_connection;
  sigc::connection volume_connection;
  sigc::connection mute_connection;
  sigc::connection blocklist_connection;
  sigc::connection timeout_connection;

  PipeManager* pm = nullptr;

  void init_widgets();

  void connect_signals();

  auto on_enable_app(bool state) -> bool;

  void on_volume_changed();

  void on_mute();

  auto float_to_localized_string(const float& value, const int& places) -> std::string;
};

#endif
