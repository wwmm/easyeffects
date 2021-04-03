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

#include "plot.hpp"

Plot::Plot(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::DrawingArea(cobject),
      controller_motion(Gtk::EventControllerMotion::create()),
      background_color(0.0, 0.0, 0.0, 1.0),
      color(1.0, 1.0, 1.0, 1.0),
      color_axis_labels(1.0, 1.0, 1.0, 1.0) {
  set_draw_func(sigc::mem_fun(*this, &Plot::on_draw));

  // signals connection

  add_controller(controller_motion);

  controller_motion->signal_motion().connect([=, this](const double& x, const double& y) {
    int width = get_width();
    int height = get_height();
    int usable_height = height - x_axis_height;

    if (y < usable_height) {
      switch (plot_scale) {
        case PlotScale::logarithmic: {
          double x_min_log = log10(x_min);
          double x_max_log = log10(x_max);

          double mouse_x_log = x / static_cast<double>(width) * (x_max_log - x_min_log) + x_min_log;

          mouse_x = std::pow(10.0, mouse_x_log);  // exp10 does not exist on FreeBSD

          mouse_y = y * (y_max - y_min) / usable_height;

          break;
        }
        case PlotScale::linear: {
          mouse_x = x / static_cast<double>(width) * (x_max - x_min) + x_min;

          mouse_y = y * (y_max - y_min) / usable_height;

          break;
        }
      }

      queue_draw();
    }
  });
}

Plot::~Plot() {
  util::debug(log_tag + "destroyed");
}

auto Plot::add_to_box(Gtk::Box* box) -> Plot* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/pulseeffects/ui/plot.ui");

  auto* ui = Gtk::Builder::get_widget_derived<Plot>(builder, "drawing_area");

  box->append(*ui);

  return ui;
}

void Plot::set_plot_type(const PlotType& value) {
  plot_type = value;
}

void Plot::set_plot_scale(const PlotScale& value) {
  plot_scale = value;
}

void Plot::set_data(const std::vector<float>& x, const std::vector<float>& y) {
  original_x = x;
  original_y = y;

  init_axes();

  queue_draw();
}

void Plot::set_n_points(const uint& value) {
  n_points = value;
}

void Plot::set_x_range(const float& min, const float& max) {
  x_min = min;
  x_max = max;
}

void Plot::set_y_range(const float& min, const float& max) {
  y_min = min;
  y_max = max;
}

void Plot::init_axes() {
  slice_x.resize(0);
  slice_y.resize(0);

  x_axis.resize(0);
  y_axis.resize(0);

  for (const auto& v : original_x) {
    if (v >= x_min && v <= x_max) {
      slice_x.emplace_back(v);
    }
  }

  for (const auto& v : original_y) {
    if (v >= y_min && v <= y_max) {
      slice_y.emplace_back(v);
    }
  }

  x_axis = slice_x;
  y_axis = slice_y;

  // making each y value a number between 0 and 1

  std::ranges::for_each(y_axis, [&](auto& v) { v = (v - y_min) / (y_max - y_min); });
}

void Plot::set_normalize_y(const bool& v) {
  normalize_y = v;
}

void Plot::set_background_color(const float& r, const float& g, const float& b, const float& alpha) {
  background_color.set_rgba(r, g, b, alpha);
}

void Plot::set_color(const float& r, const float& g, const float& b, const float& alpha) {
  color.set_rgba(r, g, b, alpha);
}

void Plot::set_axis_labels_color(const float& r, const float& g, const float& b, const float& alpha) {
  color_axis_labels.set_rgba(r, g, b, alpha);
}

void Plot::set_line_width(const float& value) {
  line_width = value;
}

void Plot::set_draw_bar_border(const bool& v) {
  draw_bar_border = v;
}

void Plot::set_fill_bars(const bool& v) {
  fill_bars = v;
}

void Plot::set_n_x_labels(const int& v) {
  n_x_labels = v;
}

void Plot::on_draw(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height) {
  ctx->paint();

  auto n_points = y_axis.size();

  if (n_points > 0) {
    auto objects_x = util::linspace(line_width, static_cast<float>(width) - line_width, n_points);

    int axis_height = draw_x_labels(ctx, width, height);

    int usable_height = height - axis_height;

    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());

    switch (plot_type) {
      case PlotType::bar: {
        for (uint n = 0U; n < n_points; n++) {
          double bar_height = static_cast<double>(usable_height) * y_axis[n];

          if (draw_bar_border) {
            ctx->rectangle(objects_x[n], static_cast<double>(usable_height) - bar_height,
                           static_cast<double>(width) / n_points - line_width, bar_height);
          } else {
            ctx->rectangle(objects_x[n], static_cast<double>(usable_height) - bar_height,
                           static_cast<double>(width) / n_points, bar_height);
          }
        }

        break;
      }
      case PlotType::line: {
        ctx->move_to(0, usable_height);

        for (uint n = 0U; n < n_points - 1U; n++) {
          auto bar_height = y_axis[n] * static_cast<float>(usable_height);

          ctx->line_to(objects_x[n], static_cast<float>(usable_height) - bar_height);
        }

        ctx->line_to(width, usable_height);

        ctx->move_to(width, usable_height);

        ctx->close_path();

        break;
      }
    }

    ctx->set_line_width(line_width);

    if (fill_bars) {
      ctx->fill();
    } else {
      ctx->stroke();
    }

    if (controller_motion->contains_pointer()) {
      std::ostringstream msg;

      msg.precision(0);

      msg << std::fixed << mouse_x << " Hz, ";
      msg << std::fixed << mouse_y << " dB";

      Pango::FontDescription font;
      font.set_family("Monospace");
      font.set_weight(Pango::Weight::BOLD);

      int text_width = 0;
      int text_height = 0;
      auto layout = create_pango_layout(msg.str());
      layout->set_font_description(font);
      layout->get_pixel_size(text_width, text_height);

      ctx->move_to(static_cast<double>(static_cast<float>(width) - static_cast<float>(text_width)), 0);

      layout->show_in_cairo_context(ctx);
    }
  }
}

auto Plot::draw_x_labels(const Cairo::RefPtr<Cairo::Context>& ctx, const int& width, const int& height) -> int {
  double labels_offset = width / static_cast<double>(n_x_labels);

  std::vector<float> labels;

  switch (plot_scale) {
    case PlotScale::logarithmic: {
      labels = util::logspace(log10f(static_cast<float>(x_min)), log10f(static_cast<float>(x_max)), n_x_labels);

      break;
    }
    case PlotScale::linear: {
      labels = util::linspace(x_min, x_max, n_x_labels);

      break;
    }
  }

  ctx->set_source_rgba(color_axis_labels.get_red(), color_axis_labels.get_green(), color_axis_labels.get_blue(),
                       color_axis_labels.get_alpha());

  /*
    we stop the loop at labels.size() - 1 because there is no space left in the window to show the last label. It
    would start to be drawn at the border of the window.
  */

  for (size_t n = 0; n < labels.size() - 1; n++) {
    std::ostringstream msg;

    auto label = labels[n];

    if (label < 1000.0) {
      msg.precision(0);
      msg << std::fixed << label << "Hz";
    } else if (label > 1000.0) {
      msg.precision(1);
      msg << std::fixed << label / 1000.0 << "kHz";
    }

    Pango::FontDescription font;
    font.set_family("Monospace");
    font.set_weight(Pango::Weight::BOLD);

    int text_width = 0;
    int text_height = 0;

    auto layout = create_pango_layout(msg.str());
    layout->set_font_description(font);
    layout->get_pixel_size(text_width, text_height);

    ctx->move_to(n * labels_offset, static_cast<double>(height - text_height));

    layout->show_in_cairo_context(ctx);

    if (n == labels.size() - 2U) {
      return text_height;
    }
  }

  return 0;
}
