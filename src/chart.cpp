#include "chart.hpp"

namespace ui::chart {

using namespace std::string_literals;

auto constexpr log_tag = "chart: ";

struct _Chart {
  GtkBox parent_instance{};

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(Chart, chart, GTK_TYPE_WIDGET)

void snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
  GdkRGBA red;
  GdkRGBA green;
  GdkRGBA yellow;
  GdkRGBA blue;
  float w = 0.0F;
  float h = 0.0F;

  gdk_rgba_parse(&red, "red");
  gdk_rgba_parse(&green, "green");
  gdk_rgba_parse(&yellow, "yellow");
  gdk_rgba_parse(&blue, "blue");

  w = gtk_widget_get_width(widget) / 2.0;
  h = gtk_widget_get_height(widget) / 2.0;

  auto rect1 = GRAPHENE_RECT_INIT(0, 0, w, h);

  gtk_snapshot_append_color(snapshot, &red, &rect1);
  // gtk_snapshot_append_color(snapshot, &green, &GRAPHENE_RECT_INIT(w, 0, w, h));
  // gtk_snapshot_append_color(snapshot, &yellow, &GRAPHENE_RECT_INIT(0, h, w, h));
  // gtk_snapshot_append_color(snapshot, &blue, &GRAPHENE_RECT_INIT(w, h, w, h));
}

void chart_class_init(ChartClass* klass) {
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  widget_class->snapshot = snapshot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/chart.ui");
}

void chart_init(Chart* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

auto create() -> Chart* {
  return static_cast<Chart*>(g_object_new(EE_TYPE_CHART, nullptr));
}

}  // namespace ui::chart