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
#include <locale>
#include <memory>
#include <mutex>
#include <vector>
#include "app_info_ui.hpp"
#include "blocklist_settings_ui.hpp"
#include "pipe_manager.hpp"
#include "preset_type.hpp"
#include "spectrum_ui.hpp"
#include "util.hpp"

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

  /*
    Do not pass node_info by reference. Sometimes it dies before we use it and a segmentation fault happens
  */

  void on_app_changed(NodeInfo node_info);
  void on_app_removed(NodeInfo node_info);

  void on_new_output_level_db(const std::array<double, 2>& peak);

 protected:
  Glib::RefPtr<Gio::Settings> settings;
  Gtk::ListBox* listbox = nullptr;
  Gtk::Stack* stack = nullptr;

  Gtk::Box *apps_box = nullptr, *app_button_row = nullptr, *global_level_meter_grid = nullptr;
  Gtk::Image *app_input_icon = nullptr, *app_output_icon = nullptr, *saturation_icon = nullptr;
  Gtk::Label *global_output_level_left = nullptr, *global_output_level_right = nullptr;

  PipeManager* pm = nullptr;

  std::vector<AppInfoUi*> apps_list;
  std::vector<sigc::connection> connections;

  std::mutex apps_list_lock_guard;

  SpectrumUi* spectrum_ui = nullptr;

 private:
  Gtk::Box* placeholder_spectrum = nullptr;

  std::locale global_locale;

  template <typename T>
  auto level_to_localized_string_showpos(const T& value, const int& places) -> std::string {
    std::ostringstream msg;

    msg.imbue(global_locale);
    msg.precision(places);

    msg << ((value > 0.0) ? "+" : "") << std::fixed << value;

    return msg.str();
  }
};

#endif
