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

#ifndef SPECTRUM_UI_HPP
#define SPECTRUM_UI_HPP

#include <giomm.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include "util.hpp"

class SpectrumUi : public Gtk::Grid {
 public:
  SpectrumUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  SpectrumUi(const SpectrumUi&) = delete;
  auto operator=(const SpectrumUi&) -> SpectrumUi& = delete;
  SpectrumUi(const SpectrumUi&&) = delete;
  auto operator=(const SpectrumUi&&) -> SpectrumUi& = delete;
  ~SpectrumUi() override;

  static auto add_to_box(Gtk::Box* box) -> SpectrumUi*;

  void on_new_spectrum(const std::vector<float>& magnitudes);

  void clear_spectrum();

 private:
  std::string log_tag = "spectrum_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::DrawingArea* spectrum = nullptr;
  Gdk::RGBA color, color_frequency_axis_labels, gradient_color;

  std::vector<sigc::connection> connections;

  bool mouse_inside = false;
  int axis_height = 0;
  double mouse_intensity = 0.0, mouse_freq = 0.0;
  std::vector<float> spectrum_mag;

  auto on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool;

  // auto on_spectrum_enter_notify_event(GdkEventCrossing* event) -> bool;

  // auto on_spectrum_leave_notify_event(GdkEventCrossing* event) -> bool;

  // auto on_spectrum_motion_notify_event(GdkEventMotion* event) -> bool;

  void init_color();

  void init_frequency_labels_color();

  void init_gradient_color();

  auto draw_frequency_axis(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height) -> int;
};

#endif
