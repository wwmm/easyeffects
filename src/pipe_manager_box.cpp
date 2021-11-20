#include "pipe_manager_box.hpp"

namespace ui::pipe_manager_box {

using namespace std::string_literals;

auto constexpr log_tag = "pipe_manager_box: ";

struct _PipeManagerBox {
  GtkBox parent_instance;

  GSettings* settings;

  app::Application* application;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(PipeManagerBox, pipe_manager_box, GTK_TYPE_BOX)

void setup(PipeManagerBox* self, app::Application* application) {
  self->application = application;
}

void pipe_manager_box_class_init(PipeManagerBoxClass* klass) {
  //   auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  //   object_class->dispose = dispose;

  //   widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/pipe_manager_box.ui");

  //   gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, stack);

  //   gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, output_scrolled_window);
  //   gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, output_listview);
  //   gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, output_name);
  //   gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, output_search);
  //   gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, last_used_output);

  //   gtk_widget_class_bind_template_callback(widget_class, create_output_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, create_input_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_output_preset);
  //   gtk_widget_class_bind_template_callback(widget_class, import_input_preset);
}

void pipe_manager_box_init(PipeManagerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

auto create() -> PipeManagerBox* {
  return static_cast<PipeManagerBox*>(g_object_new(EE_TYPE_PIPE_MANAGER_BOX, nullptr));
}

}  // namespace ui::pipe_manager_box