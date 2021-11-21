#include "pipe_manager_box.hpp"

namespace ui::pipe_manager_box {

using namespace std::string_literals;

auto constexpr log_tag = "pipe_manager_box: ";

struct _PipeManagerBox {
  GtkBox parent_instance;

  AdwViewStack* stack;

  GtkSwitch *use_default_input, *use_default_output, *enable_test_signal;

  Gtk::DropDown *dropdown_input_devices, *dropdown_output_devices, *dropdown_autoloading_output_devices,
      *dropdown_autoloading_input_devices, *dropdown_autoloading_output_presets, *dropdown_autoloading_input_presets;

  GtkListView *listview_modules, *listview_clients, *listview_autoloading_output, *listview_autoloading_input;

  GtkButton *autoloading_add_input_profile, *autoloading_add_output_profile;

  GtkLabel *header_version, *library_version, *quantum, *max_quantum, *min_quantum, *server_rate;

  GtkSpinButton* spinbutton_test_signal_frequency;

  GListStore *input_devices_model, *output_devices_model;

  GtkStringList *input_presets_string_list, *output_presets_string_list;

  GSettings *sie_settings, *soe_settings;

  app::Application* application;

  TestSignals* ts;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections_sie, gconnections_soe;
};

G_DEFINE_TYPE(PipeManagerBox, pipe_manager_box, GTK_TYPE_BOX)

void on_checkbutton_channel_left(PipeManagerBox* self, GtkCheckButton* btn) {
  if (gtk_check_button_get_active(btn)) {
    self->ts->create_left_channel = true;
    self->ts->create_right_channel = false;
  }
}

void on_checkbutton_channel_right(PipeManagerBox* self, GtkCheckButton* btn) {
  if (gtk_check_button_get_active(btn)) {
    self->ts->create_left_channel = false;
    self->ts->create_right_channel = true;
  }
}

void on_checkbutton_channel_both(PipeManagerBox* self, GtkCheckButton* btn) {
  if (gtk_check_button_get_active(btn)) {
    self->ts->create_left_channel = true;
    self->ts->create_right_channel = true;
  }
}

void on_checkbutton_signal_sine(PipeManagerBox* self, GtkCheckButton* btn) {
  if (gtk_check_button_get_active(btn)) {
    self->ts->signal_type = TestSignalType::sine_wave;
    self->ts->sine_phase = 0.0F;
  }
}

void on_checkbutton_signal_gaussian(PipeManagerBox* self, GtkCheckButton* btn) {
  if (gtk_check_button_get_active(btn)) {
    self->ts->signal_type = TestSignalType::gaussian;
  }
}

void setup(PipeManagerBox* self, app::Application* application) {
  self->application = application;

  auto pm = application->pm.get();

  self->ts = new TestSignals(pm);

  for (const auto& [ts, node] : pm->node_map) {
    if (node.name == pm->ee_sink_name || node.name == pm->ee_source_name) {
      continue;
    }

    if (node.media_class == pm->media_class_sink) {
      g_list_store_append(self->output_devices_model, ui::holders::create(node));
    } else if (node.media_class == pm->media_class_source || node.media_class == pm->media_class_virtual_source) {
      g_list_store_append(self->input_devices_model, ui::holders::create(node));
    }
  }

  gtk_label_set_text(self->header_version, pm->header_version.c_str());
  gtk_label_set_text(self->library_version, pm->library_version.c_str());
  gtk_label_set_text(self->server_rate, fmt::format("{0:d} Hz", std::stoi(pm->default_clock_rate)).c_str());
  gtk_label_set_text(self->min_quantum, pm->default_min_quantum.c_str());
  gtk_label_set_text(self->max_quantum, pm->default_max_quantum.c_str());
  gtk_label_set_text(self->quantum, pm->default_quantum.c_str());
}

void dispose(GObject* object) {
  auto* self = EE_PIPE_MANAGER_BOX(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections_sie) {
    g_signal_handler_disconnect(self->sie_settings, handler_id);
  }

  for (auto& handler_id : self->gconnections_soe) {
    g_signal_handler_disconnect(self->soe_settings, handler_id);
  }

  self->connections.clear();
  self->gconnections_sie.clear();
  self->gconnections_soe.clear();

  g_object_unref(self->sie_settings);
  g_object_unref(self->soe_settings);

  delete self->ts;

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(pipe_manager_box_parent_class)->dispose(object);
}

void pipe_manager_box_class_init(PipeManagerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  //   widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/pipe_manager_box.ui");

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, stack);

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, use_default_input);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, use_default_output);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, enable_test_signal);

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, dropdown_input_devices);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, dropdown_output_devices);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, dropdown_autoloading_input_devices);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, dropdown_autoloading_output_devices);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, dropdown_autoloading_input_presets);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, dropdown_autoloading_output_presets);

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, listview_modules);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, listview_clients);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, listview_autoloading_input);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, listview_autoloading_output);

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, autoloading_add_input_profile);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, autoloading_add_output_profile);

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, header_version);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, library_version);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, quantum);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, max_quantum);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, min_quantum);
  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, server_rate);

  gtk_widget_class_bind_template_child(widget_class, PipeManagerBox, spinbutton_test_signal_frequency);

  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_channel_left);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_channel_right);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_channel_both);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_signal_sine);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_signal_gaussian);
}

void pipe_manager_box_init(PipeManagerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->input_presets_string_list = gtk_string_list_new(nullptr);
  self->output_presets_string_list = gtk_string_list_new(nullptr);

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->output_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  self->sie_settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");
  self->soe_settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

  prepare_spinbutton<"Hz">(self->spinbutton_test_signal_frequency);

  g_signal_connect(self->spinbutton_test_signal_frequency, "value-changed",
                   G_CALLBACK(+[](GtkSpinButton* btn, PipeManagerBox* self) {
                     self->ts->set_frequency(static_cast<float>(gtk_spin_button_get_value(btn)));
                   }),
                   self);
}

auto create() -> PipeManagerBox* {
  return static_cast<PipeManagerBox*>(g_object_new(EE_TYPE_PIPE_MANAGER_BOX, nullptr));
}

}  // namespace ui::pipe_manager_box