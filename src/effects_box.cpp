#include "effects_box.hpp"

namespace ui::effects_box {

using namespace std::string_literals;

auto constexpr log_tag = "effects_box: ";

struct _EffectsBox {
  GtkBox parent_instance{};

  AdwViewStack* stack;

  ui::chart::Chart* spectrum_chart;

  GSettings *settings, *settings_spectrum, *app_settings;

  app::Application* application;

  EffectsBase* effects_base;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections_spectrum;
};

G_DEFINE_TYPE(EffectsBox, effects_box, GTK_TYPE_BOX)

void setup_spectrum(EffectsBox* self) {
  self->gconnections_spectrum.push_back(g_signal_connect(
      self->settings_spectrum, "changed::color", G_CALLBACK((+[](GSettings* settings, char* key, EffectsBox* self) {
        auto color = util::gsettings_get_color(self->settings_spectrum, key);
      })),
      self));
}

void setup(EffectsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->effects_base = static_cast<EffectsBase*>(self->application->sie.get());

      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      break;
    }
    case PipelineType::output: {
      self->effects_base = static_cast<EffectsBase*>(self->application->soe.get());

      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      break;
    }
  }

  self->effects_base->spectrum->power.connect([=](uint rate, uint n_points, std::vector<float> data) {
    // util::warning(std::to_string(n_points));
  });

  self->effects_base->spectrum->post_messages = gtk_widget_get_visible(GTK_WIDGET(self->spectrum_chart)) != 0;

  self->effects_base->spectrum->bypass = false;
}

void dispose(GObject* object) {
  auto* self = EE_EFFECTS_BOX(object);

  self->effects_base->spectrum->post_messages = false;

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections_spectrum) {
    g_signal_handler_disconnect(self->settings_spectrum, handler_id);
  }

  util::debug(log_tag + "destroyed"s);

  G_OBJECT_CLASS(effects_box_parent_class)->dispose(object);
}

void effects_box_class_init(EffectsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

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

  self->app_settings = g_settings_new("com.github.wwmm.easyeffects");
  self->settings_spectrum = g_settings_new("com.github.wwmm.easyeffects.spectrum");

  self->spectrum_chart = ui::chart::create();

  gtk_box_insert_child_after(GTK_BOX(self), GTK_WIDGET(self->spectrum_chart), nullptr);
}

auto create() -> EffectsBox* {
  return static_cast<EffectsBox*>(g_object_new(EE_TYPE_EFFECTS_BOX, nullptr));
}

}  // namespace ui::effects_box