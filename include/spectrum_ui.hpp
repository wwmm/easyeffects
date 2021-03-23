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
#include <gtkmm.h>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include "util.hpp"

class SpectrumUi : public Gtk::DrawingArea {
 public:
  SpectrumUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  SpectrumUi(const SpectrumUi&) = delete;
  auto operator=(const SpectrumUi&) -> SpectrumUi& = delete;
  SpectrumUi(const SpectrumUi&&) = delete;
  auto operator=(const SpectrumUi&&) -> SpectrumUi& = delete;
  ~SpectrumUi() override;

  static auto add_to_box(Gtk::Box* box) -> SpectrumUi*;

  void on_new_spectrum(const uint& rate, const uint& n_bands, const std::vector<float>& magnitudes);

  void clear_spectrum();

 private:
  std::string log_tag = "spectrum_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Glib::RefPtr<Gtk::EventControllerMotion> controller_motion;

  Gdk::RGBA color, color_frequency_axis_labels, gradient_color;

  std::vector<sigc::connection> connections;

  int axis_height = 0;
  double mouse_intensity = 0.0, mouse_freq = 0.0;

  uint rate = 0, n_bands = 0;
  float spline_f0 = 0.0F, spline_df = 0.0F;

  std::vector<float> spectrum_mag, spectrum_freqs, spectrum_x_axis;

  void on_draw(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height);

  void init_color();

  void init_frequency_axis();

  void init_frequency_labels_color();

  void init_gradient_color();

  auto draw_frequency_axis(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height) -> int;
};

#endif
