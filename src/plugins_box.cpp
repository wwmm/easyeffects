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

#include "plugins_box.hpp"

namespace ui::plugins_box {

using namespace std::string_literals;

auto constexpr log_tag = "plugins_box: ";

struct _PluginsBox {
  GtkBox parent_instance{};

  app::Application* application;

  bool schedule_signal_idle;

  PipelineType pipeline_type;

  std::vector<sigc::connection> connections;
};

G_DEFINE_TYPE(PluginsBox, plugins_box, GTK_TYPE_BOX)

void setup(PluginsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;
  self->pipeline_type = pipeline_type;
}

void realize(GtkWidget* widget) {
  auto* self = EE_PLUGINS_BOX(widget);

  self->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(plugins_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_PLUGINS_BOX(widget);

  self->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(plugins_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PLUGINS_BOX(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed"s);

  G_OBJECT_CLASS(plugins_box_parent_class)->dispose(object);
}

void plugins_box_class_init(PluginsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->realize = realize;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/plugins_box.ui");

  //   gtk_widget_class_bind_template_child(widget_class, PluginsBox, stack);
  //   gtk_widget_class_bind_template_child(widget_class, PluginsBox, device_state);
  //   gtk_widget_class_bind_template_child(widget_class, PluginsBox, latency_status);
  //   gtk_widget_class_bind_template_child(widget_class, PluginsBox, label_global_output_level_left);
  //   gtk_widget_class_bind_template_child(widget_class, PluginsBox, label_global_output_level_right);
}

void plugins_box_init(PluginsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->schedule_signal_idle = false;
}

auto create() -> PluginsBox* {
  return static_cast<PluginsBox*>(g_object_new(EE_TYPE_PLUGINS_BOX, nullptr));
}

}  // namespace ui::plugins_box