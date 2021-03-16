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

#ifndef SPECTRUM_SETTINGS_UI_HPP
#define SPECTRUM_SETTINGS_UI_HPP

#include <giomm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <cstring>
#include "application.hpp"
#include "util.hpp"

class SpectrumSettingsUi : public Gtk::Box {
 public:
  SpectrumSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Application* application);
  SpectrumSettingsUi(const SpectrumSettingsUi&) = delete;
  auto operator=(const SpectrumSettingsUi&) -> SpectrumSettingsUi& = delete;
  SpectrumSettingsUi(const SpectrumSettingsUi&&) = delete;
  auto operator=(const SpectrumSettingsUi&&) -> SpectrumSettingsUi& = delete;
  ~SpectrumSettingsUi() override;

  static void add_to_stack(Gtk::Stack* stack, Application* app);

 private:
  std::string log_tag = "spectrum_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app = nullptr;

  Gtk::Switch *show = nullptr, *use_custom_color = nullptr, *fill = nullptr, *show_bar_border = nullptr,
              *use_gradient = nullptr;

  Gtk::ColorButton *spectrum_color_button = nullptr, *gradient_color_button = nullptr, *axis_color_button = nullptr;

  Gtk::ComboBoxText* spectrum_type = nullptr;

  Gtk::SpinButton *n_points = nullptr, *height = nullptr, *line_width = nullptr, *minimum_frequency = nullptr,
                  *maximum_frequency = nullptr;

  std::vector<sigc::connection> connections;

  auto on_use_custom_color(bool state) -> bool;
};

#endif
