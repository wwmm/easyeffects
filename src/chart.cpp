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

#include "chart.hpp"

namespace ui::chart {

using namespace std::string_literals;

auto constexpr log_tag = "chart_box: ";

/*
  It is super weird having to do this... I know... But for some reason GTK does not destroy the widget structure even
  when it is removed from its parent container the cooresponding reference count goes to zero... Who knows why...
  THe problem is that because of this the vector destructors are never called. And whenever the convolver is removed
  and added again our memory usage gets big quickly. This Data structure is a workaround for that. By calling delete on
  it when the widget is finalized we force that memory to be freed. It is unbelievable I am being forced to do
  something like this...
*/

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  bool draw_bar_border, fill_bars, is_visible;

  int x_axis_height, n_x_decimals, n_y_decimals;

  double mouse_y, mouse_x, margin, line_width;

  float x_min, x_max, y_min, y_max;

  ChartType chart_type;

  ChartScale chart_scale;

  GdkRGBA background_color, color, color_axis_labels, gradient_color;

  std::string x_unit, y_unit;

  std::vector<float> original_x, original_y, y_axis, x_axis;
};

struct _Chart {
  GtkBox parent_instance;

  GtkEventController* controller_motion;

  Data* data;
};

G_DEFINE_TYPE(Chart, chart, GTK_TYPE_WIDGET)

void set_plot_type(Chart* self, const ChartType& value) {
  self->data->chart_type = value;
}

void set_plot_scale(Chart* self, const ChartScale& value) {
  self->data->chart_scale = value;
}

void set_background_color(Chart* self, GdkRGBA color) {
  self->data->background_color = color;
}

void set_color(Chart* self, GdkRGBA color) {
  self->data->color = color;
}

void set_axis_labels_color(Chart* self, GdkRGBA color) {
  self->data->color_axis_labels = color;
}

void set_line_width(Chart* self, const double& value) {
  self->data->line_width = value;
}

void set_draw_bar_border(Chart* self, const bool& v) {
  self->data->draw_bar_border = v;
}

void set_fill_bars(Chart* self, const bool& v) {
  self->data->fill_bars = v;
}

void set_n_x_decimals(Chart* self, const int& v) {
  self->data->n_x_decimals = v;
}

void set_n_y_decimals(Chart* self, const int& v) {
  self->data->n_y_decimals = v;
}

void set_x_unit(Chart* self, const std::string& value) {
  self->data->x_unit = value;
}

void set_y_unit(Chart* self, const std::string& value) {
  self->data->y_unit = value;
}

void set_margin(Chart* self, const double& v) {
  self->data->margin = v;
}

auto get_is_visible(Chart* self) -> bool {
  return self->data->is_visible;
}

void init_axes(Chart* self) {
  if (self->data->original_x.empty() || self->data->original_y.empty()) {
    return;
  }

  self->data->x_min = std::ranges::min(self->data->original_x);
  self->data->x_max = std::ranges::max(self->data->original_x);

  self->data->y_min = std::ranges::min(self->data->original_y);
  self->data->y_max = std::ranges::max(self->data->original_y);

  self->data->x_axis.resize(0);
  self->data->y_axis.resize(0);

  for (const auto& v : self->data->original_x) {
    if (v >= self->data->x_min && v <= self->data->x_max) {
      self->data->x_axis.push_back(v);
    }
  }

  for (const auto& v : self->data->original_y) {
    if (v >= self->data->y_min && v <= self->data->y_max) {
      self->data->y_axis.push_back(v);
    }
  }

  // making each y value a number between 0 and 1

  std::ranges::for_each(self->data->y_axis,
                        [&](auto& v) { v = (v - self->data->y_min) / (self->data->y_max - self->data->y_min); });
}

void set_data(Chart* self, const std::vector<float>& x, const std::vector<float>& y) {
  if (self == nullptr) {
    return;
  }

  if (!self->data->is_visible) {
    return;
  }

  self->data->original_x = x;
  self->data->original_y = y;

  init_axes(self);

  gtk_widget_queue_draw(GTK_WIDGET(self));
}

void on_pointer_motion(GtkEventControllerMotion* controller, double x, double y, Chart* self) {
  const int width = gtk_widget_get_allocated_width(GTK_WIDGET(self));
  const int height = gtk_widget_get_allocated_height(GTK_WIDGET(self));

  const auto usable_height = height - self->data->margin * height - self->data->x_axis_height;

  if (y < height - self->data->x_axis_height && y > self->data->margin * height && x > self->data->margin * width &&
      x < width - self->data->margin * width) {
    switch (self->data->chart_scale) {
      case ChartScale::logarithmic: {
        const double& x_min_log = std::log10(self->data->x_min);
        const double& x_max_log = std::log10(self->data->x_max);

        const double& mouse_x_log = (x - self->data->margin * width) /
                                        static_cast<double>(width - 2 * self->data->margin * width) *
                                        (x_max_log - x_min_log) +
                                    x_min_log;

        self->data->mouse_x = std::pow(10.0, mouse_x_log);  // exp10 does not exist on FreeBSD

        self->data->mouse_y =
            (usable_height - y) / usable_height * (self->data->y_max - self->data->y_min) + self->data->y_min;

        break;
      }
      case ChartScale::linear: {
        self->data->mouse_x = (x - self->data->margin * width) /
                                  static_cast<double>(width - 2 * self->data->margin * width) *
                                  (self->data->x_max - self->data->x_min) +
                              self->data->x_min;

        self->data->mouse_y = (self->data->y_max - self->data->y_min) * (usable_height - y) / usable_height;

        break;
      }
    }

    gtk_widget_queue_draw(GTK_WIDGET(self));
  }
}

auto draw_x_labels(Chart* self, cairo_t* ctx, const int& width, const int& height) -> int {
  /*
     Initial value for the offset between labels. For it has been found based on trial and error. It would be good
     to have a better procedure to estimate the "good" separation value between labels
  */

  double labels_offset = 120;

  int n_x_labels = static_cast<int>(std::ceil((width - 2 * self->data->margin * width) / labels_offset)) + 1;

  if (n_x_labels < 2) {
    return 0;
  }

  /*
    Correcting the offset based on the final n_x_labels value
  */

  labels_offset = (width - 2 * self->data->margin * width) / static_cast<double>(n_x_labels - 1);

  std::vector<float> labels;

  switch (self->data->chart_scale) {
    case ChartScale::logarithmic: {
      labels = util::logspace(self->data->x_min, self->data->x_max, n_x_labels);

      break;
    }
    case ChartScale::linear: {
      labels = util::linspace(self->data->x_min, self->data->x_max, n_x_labels);

      break;
    }
  }

  cairo_set_source_rgba(ctx, self->data->color_axis_labels.red, self->data->color_axis_labels.green,
                        self->data->color_axis_labels.blue, self->data->color_axis_labels.alpha);

  /*
    There is no space left in the window to show the last label. So we skip it
  */

  for (size_t n = 0U; n < labels.size() - 1; n++) {
    const auto msg = fmt::format("{0:.{1}f} {2}", labels[n], self->data->n_x_decimals, self->data->x_unit);

    auto* layout = gtk_widget_create_pango_layout(GTK_WIDGET(self), msg.c_str());

    auto* description = pango_font_description_from_string("monospace bold");

    pango_layout_set_font_description(layout, description);
    pango_font_description_free(description);

    int text_width = 0;
    int text_height = 0;

    pango_layout_get_pixel_size(layout, &text_width, &text_height);

    cairo_move_to(ctx, self->data->margin * width + static_cast<double>(n) * labels_offset,
                  static_cast<double>(height - text_height));

    pango_cairo_show_layout(ctx, layout);

    g_object_unref(layout);

    if (n == labels.size() - 2U) {
      return text_height;
    }
  }

  return 0;
}

void snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
  auto* self = EE_CHART(widget);

  int width = gtk_widget_get_width(widget);
  int height = gtk_widget_get_height(widget);

  auto widget_rectangle = GRAPHENE_RECT_INIT(0.0F, 0.0F, static_cast<float>(width), static_cast<float>(height));

  gtk_snapshot_append_color(snapshot, &self->data->background_color, &widget_rectangle);

  auto* ctx = gtk_snapshot_append_cairo(snapshot, &widget_rectangle);

  if (const auto n_points = self->data->y_axis.size(); n_points > 0) {
    const auto objects_x = util::linspace(
        static_cast<float>(self->data->line_width + self->data->margin * width),
        static_cast<float>(static_cast<float>(width) - self->data->line_width - self->data->margin * width), n_points);

    if (objects_x.empty()) {
      return;
    }

    self->data->x_axis_height = draw_x_labels(self, ctx, width, height);

    int usable_height = static_cast<int>(height - self->data->margin * height) - self->data->x_axis_height;

    cairo_set_source_rgba(ctx, self->data->color.red, self->data->color.green, self->data->color.blue,
                          self->data->color.alpha);

    switch (self->data->chart_type) {
      case ChartType::bar: {
        for (uint n = 0U; n < n_points; n++) {
          double bar_height = static_cast<double>(usable_height) * self->data->y_axis[n];

          if (self->data->draw_bar_border) {
            cairo_rectangle(
                ctx, objects_x[n], self->data->margin * height + static_cast<double>(usable_height) - bar_height,
                static_cast<double>(width) / static_cast<double>(n_points) - self->data->line_width, bar_height);
          } else {
            cairo_rectangle(ctx, objects_x[n],
                            self->data->margin * height + static_cast<double>(usable_height) - bar_height,
                            static_cast<double>(width) / static_cast<double>(n_points), bar_height);
          }
        }

        break;
      }
      case ChartType::line: {
        if (self->data->fill_bars) {
          cairo_move_to(ctx, self->data->margin * width,
                        self->data->margin * height + static_cast<float>(usable_height));
        } else {
          const auto point_height = self->data->y_axis.front() * static_cast<float>(usable_height);

          cairo_move_to(ctx, objects_x.front(),
                        self->data->margin * height + static_cast<float>(usable_height) - point_height);
        }

        for (uint n = 0U; n < n_points - 1U; n++) {
          const auto next_point_height = self->data->y_axis[n + 1] * static_cast<float>(usable_height);

          cairo_line_to(ctx, objects_x[n + 1],
                        self->data->margin * height + static_cast<float>(usable_height) - next_point_height);
        }

        if (self->data->fill_bars) {
          cairo_line_to(ctx, objects_x.back(), self->data->margin * height + static_cast<float>(usable_height));

          cairo_move_to(ctx, objects_x.back(), self->data->margin * height + static_cast<float>(usable_height));

          cairo_close_path(ctx);
        }

        break;
      }
    }

    cairo_set_line_width(ctx, self->data->line_width);

    if (self->data->fill_bars) {
      cairo_fill(ctx);
    } else {
      cairo_stroke(ctx);
    }

    if (gtk_event_controller_motion_contains_pointer(GTK_EVENT_CONTROLLER_MOTION(self->controller_motion)) != 0) {
      const auto msg =
          fmt::format("x = {0:.{1}f} {2} y = {3:.{4}f} {5}", self->data->mouse_x, self->data->n_x_decimals,
                      self->data->x_unit, self->data->mouse_y, self->data->n_y_decimals, self->data->y_unit);

      auto* layout = gtk_widget_create_pango_layout(GTK_WIDGET(self), msg.c_str());

      auto* description = pango_font_description_from_string("monospace bold");

      pango_layout_set_font_description(layout, description);
      pango_font_description_free(description);

      int text_width = 0;
      int text_height = 0;

      pango_layout_get_pixel_size(layout, &text_width, &text_height);

      cairo_move_to(ctx, static_cast<double>(static_cast<float>(width) - static_cast<float>(text_width)), 0);

      pango_cairo_show_layout(ctx, layout);

      g_object_unref(layout);
    }
  }

  cairo_destroy(ctx);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_CHART(widget);

  self->data->is_visible = false;

  GTK_WIDGET_CLASS(chart_parent_class)->unmap(widget);
}

void finalize(GObject* object) {
  auto* self = EE_CHART(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(chart_parent_class)->finalize(object);
}

void chart_class_init(ChartClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->finalize = finalize;

  widget_class->snapshot = snapshot;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/chart.ui");
}

void chart_init(Chart* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->data->draw_bar_border = true;
  self->data->fill_bars = true;
  self->data->is_visible = true;
  self->data->x_axis_height = 0;
  self->data->n_x_decimals = 1;
  self->data->n_y_decimals = 1;
  self->data->line_width = 2.0F;
  self->data->margin = 0.02;

  self->data->x_min = 0.0F;
  self->data->y_min = 0.0F;
  self->data->x_max = 1.0F;
  self->data->y_max = 1.0F;

  self->data->background_color = GdkRGBA{0.0F, 0.0F, 0.0F, 1.0F};
  self->data->color = GdkRGBA{1.0F, 1.0F, 1.0F, 1.0F};
  self->data->color_axis_labels = GdkRGBA{1.0F, 1.0F, 1.0F, 1.0F};
  self->data->gradient_color = GdkRGBA{1.0F, 1.0F, 1.0F, 1.0F};

  self->data->chart_type = ChartType::bar;
  self->data->chart_scale = ChartScale::logarithmic;

  self->controller_motion = gtk_event_controller_motion_new();

  g_signal_connect(self->controller_motion, "motion", G_CALLBACK(on_pointer_motion), self);

  g_signal_connect(GTK_WIDGET(self), "hide",
                   G_CALLBACK(+[](GtkWidget* widget, Chart* self) { self->data->is_visible = false; }), self);

  g_signal_connect(GTK_WIDGET(self), "show",
                   G_CALLBACK(+[](GtkWidget* widget, Chart* self) { self->data->is_visible = true; }), self);

  gtk_widget_add_controller(GTK_WIDGET(self), self->controller_motion);
}

auto create() -> Chart* {
  return static_cast<Chart*>(g_object_new(EE_TYPE_CHART, nullptr));
}

}  // namespace ui::chart