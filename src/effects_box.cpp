#include "effects_box.hpp"

namespace ui::effects_box {

using namespace std::string_literals;

auto constexpr log_tag = "effects_box: ";

struct _EffectsBox {
  GtkBox parent_instance{};

  AdwViewStack* stack = nullptr;

  GSettings* settings = nullptr;

  app::Application* application = nullptr;

  EffectsBase* effects_base = nullptr;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(EffectsBox, effects_box, GTK_TYPE_BOX)

void setup(EffectsBox* self, app::Application* application) {
  self->application = application;
}

void effects_box_class_init(EffectsBoxClass* klass) {
  //   auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  //   object_class->dispose = dispose;

  //   widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/effects_box.ui");

  gtk_widget_class_bind_template_child(widget_class, EffectsBox, stack);

  //   gtk_widget_class_bind_template_child(widget_class, EffectsBox, output_scrolled_window);
  //   gtk_widget_class_bind_template_child(widget_class, EffectsBox, output_listview);
  //   gtk_widget_class_bind_template_child(widget_class, EffectsBox, output_name);
  //   gtk_widget_class_bind_template_child(widget_class, EffectsBox, output_search);
  //   gtk_widget_class_bind_template_child(widget_class, EffectsBox, last_used_output);

  //   gtk_widget_class_bind_template_callback(widget_class, create_output_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, create_input_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_output_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_input_preset);
}

void effects_box_init(EffectsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

auto create() -> EffectsBox* {
  return static_cast<EffectsBox*>(g_object_new(EE_TYPE_EFFECTS_BOX, nullptr));
}

}  // namespace ui::effects_box