/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#ifndef PLOT_UI_HPP
#define PLOT_UI_HPP

#include <gtkmm.h>
#include <iomanip>
#include <ranges>
#include "util.hpp"

enum class PlotType { bar, line };

enum class PlotScale { linear, logarithmic };

class Plot {
 public:
  Plot(Gtk::DrawingArea* drawing_area);
  Plot(const Plot&) = delete;
  auto operator=(const Plot&) -> Plot& = delete;
  Plot(const Plot&&) = delete;
  auto operator=(const Plot&&) -> Plot& = delete;
  ~Plot();

  void set_plot_type(const PlotType& value);

  void set_plot_scale(const PlotScale& value);

  void set_data(const std::vector<float>& x, const std::vector<float>& y);

  void set_background_color(const float& r, const float& g, const float& b, const float& alpha);

  void set_color(const float& r, const float& g, const float& b, const float& alpha);

  void set_axis_labels_color(const float& r, const float& g, const float& b, const float& alpha);

  void set_line_width(const float& value);

  void set_draw_bar_border(const bool& v);

  void set_fill_bars(const bool& v);

  void set_n_x_labels(const int& v);

  void set_n_x_decimals(const int& v);

  void set_n_y_decimals(const int& v);

  void set_x_unit(const Glib::ustring& value);

  void set_y_unit(const Glib::ustring& value);

 private:
  inline static const std::string log_tag = "plot: ";

  Gtk::DrawingArea* da = nullptr;

  Glib::RefPtr<Gtk::EventControllerMotion> controller_motion;

  Gdk::RGBA background_color, color, color_axis_labels, gradient_color;

  bool draw_bar_border = true;
  bool fill_bars = true;

  int x_axis_height = 0;
  int n_x_labels = 10;
  int n_x_decimals = 1;
  int n_y_decimals = 1;

  double mouse_y = 0.0, mouse_x = 0.0;

  float x_min = 0.0F, x_max = 1.0F;
  float y_min = 0.0F, y_max = 1.0F;

  float line_width = 2.0F;

  PlotType plot_type = PlotType::bar;

  PlotScale plot_scale = PlotScale::logarithmic;

  Glib::ustring x_unit, y_unit;

  std::vector<float> original_x, original_y;
  std::vector<float> y_axis, x_axis;

  void init_axes();

  void on_draw(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height);

  auto draw_x_labels(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height) -> int;
};

#endif
