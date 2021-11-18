#include "apps_box.hpp"

namespace ui::apps_box {

using namespace std::string_literals;

auto constexpr log_tag = "apps_box: ";

struct _AppsBox {
  GtkBox parent_instance{};

  app::Application* application;

  bool schedule_signal_idle;

  std::vector<sigc::connection> connections;
};

G_DEFINE_TYPE(AppsBox, apps_box, GTK_TYPE_BOX)

void setup(AppsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;
}

void realize(GtkWidget* widget) {
  auto* self = EE_APPS_BOX(widget);

  self->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(apps_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_APPS_BOX(widget);

  self->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(apps_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_APPS_BOX(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed"s);

  G_OBJECT_CLASS(apps_box_parent_class)->dispose(object);
}

void apps_box_class_init(AppsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->realize = realize;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/apps_box.ui");

  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, stack);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, device_state);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, latency_status);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, label_global_output_level_left);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, label_global_output_level_right);
}

void apps_box_init(AppsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->schedule_signal_idle = false;
}

auto create() -> AppsBox* {
  return static_cast<AppsBox*>(g_object_new(EE_TYPE_APPS_BOX, nullptr));
}

}  // namespace ui::apps_box