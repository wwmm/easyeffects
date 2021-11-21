#include "pipe_manager_box.hpp"

namespace ui::pipe_manager_box {

using namespace std::string_literals;

auto constexpr log_tag = "pipe_manager_box: ";

struct _PipeManagerBox {
  GtkBox parent_instance;

  GtkSwitch *use_default_input, *use_default_output, *enable_test_signal;

  Gtk::DropDown *dropdown_input_devices, *dropdown_output_devices, *dropdown_autoloading_output_devices,
      *dropdown_autoloading_input_devices, *dropdown_autoloading_output_presets, *dropdown_autoloading_input_presets;

  GtkListView *listview_modules, *listview_clients, *listview_autoloading_output, *listview_autoloading_input;

  GtkButton *autoloading_add_input_profile, *autoloading_add_output_profile;

  GtkLabel *header_version, *library_version, *quantum, *max_quantum, *min_quantum, *server_rate;

  GtkSpinButton* spinbutton_test_signal_frequency;

  GListStore *input_devices_model, *output_devices_model, *modules_model, *clients_model, *autoloading_input_model,
      *autoloading_output_model;

  GtkStringList *input_presets_string_list, *output_presets_string_list;

  GSettings *sie_settings, *soe_settings;

  app::Application* application;

  TestSignals* ts;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections_sie, gconnections_soe;
};

G_DEFINE_TYPE(PipeManagerBox, pipe_manager_box, GTK_TYPE_BOX)

void on_enable_test_signal(PipeManagerBox* self, gboolean state, GtkSwitch* btn) {
  self->ts->set_state(state != 0);
}

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

void update_modules_info(PipeManagerBox* self) {
  std::vector<ui::holders::ModuleInfoHolder*> values;

  for (const auto& info : self->application->pm->list_modules) {
    values.push_back(ui::holders::create(info));
  }

  g_list_store_splice(self->modules_model, 0, g_list_model_get_n_items(G_LIST_MODEL(self->modules_model)),
                      (gpointer*)(values.data()), values.size());
}

void update_clients_info(PipeManagerBox* self) {
  std::vector<ui::holders::ClientInfoHolder*> values;

  for (const auto& info : self->application->pm->list_clients) {
    values.push_back(ui::holders::create(info));
  }

  g_list_store_splice(self->clients_model, 0, g_list_model_get_n_items(G_LIST_MODEL(self->clients_model)),
                      (gpointer*)(values.data()), values.size());
}

void on_stack_visible_child_changed(PipeManagerBox* self, GParamSpec* pspec, GtkWidget* stack) {
  if (const auto name = gtk_stack_get_visible_child_name(GTK_STACK(stack)); g_strcmp0(name, "page_modules") == 0) {
    update_modules_info(self);
  } else if (g_strcmp0(name, "page_clients") == 0) {
    update_clients_info(self);
  }
}

void setup_listview_modules(PipeManagerBox* self) {
  auto* selection = gtk_no_selection_new(G_LIST_MODEL(self->modules_model));

  gtk_list_view_set_model(self->listview_modules, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);

  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(factory, "setup",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PipeManagerBox* self) {
                     auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/module_info.ui");

                     auto* top_box = gtk_builder_get_object(builder, "top_box");

                     g_object_set_data(G_OBJECT(item), "id", gtk_builder_get_object(builder, "id"));
                     g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
                     g_object_set_data(G_OBJECT(item), "description", gtk_builder_get_object(builder, "description"));

                     gtk_list_item_set_activatable(item, 0);
                     gtk_list_item_set_child(item, GTK_WIDGET(top_box));

                     g_object_unref(builder);
                   }),
                   self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PipeManagerBox* self) {
                     auto* id = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "id"));
                     auto* name = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* description = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "description"));

                     auto* holder = static_cast<ui::holders::ModuleInfoHolder*>(gtk_list_item_get_item(item));

                     gtk_label_set_text(id, std::to_string(holder->id).c_str());
                     gtk_label_set_text(name, holder->name.c_str());
                     gtk_label_set_text(description, holder->description.c_str());
                   }),
                   self);

  gtk_list_view_set_factory(self->listview_modules, factory);

  g_object_unref(factory);
}

void setup_listview_clients(PipeManagerBox* self) {
  auto* selection = gtk_no_selection_new(G_LIST_MODEL(self->clients_model));

  gtk_list_view_set_model(self->listview_clients, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);

  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(factory, "setup",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PipeManagerBox* self) {
                     auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/client_info.ui");

                     auto* top_box = gtk_builder_get_object(builder, "top_box");

                     g_object_set_data(G_OBJECT(item), "id", gtk_builder_get_object(builder, "id"));
                     g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
                     g_object_set_data(G_OBJECT(item), "api", gtk_builder_get_object(builder, "api"));
                     g_object_set_data(G_OBJECT(item), "access", gtk_builder_get_object(builder, "access"));

                     gtk_list_item_set_activatable(item, 0);
                     gtk_list_item_set_child(item, GTK_WIDGET(top_box));

                     g_object_unref(builder);
                   }),
                   self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PipeManagerBox* self) {
                     auto* id = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "id"));
                     auto* name = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* api = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "api"));
                     auto* access = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "access"));

                     auto* holder = static_cast<ui::holders::ClientInfoHolder*>(gtk_list_item_get_item(item));

                     gtk_label_set_text(id, std::to_string(holder->id).c_str());
                     gtk_label_set_text(name, holder->name.c_str());
                     gtk_label_set_text(api, holder->api.c_str());
                     gtk_label_set_text(access, holder->access.c_str());
                   }),
                   self);

  gtk_list_view_set_factory(self->listview_clients, factory);

  g_object_unref(factory);
}

template <PresetType preset_type>
void setup_listview_autoloading(PipeManagerBox* self) {
  GListStore* model;
  GtkListView* listview;

  if constexpr (preset_type == PresetType::output) {
    model = self->autoloading_output_model;

    listview = self->listview_autoloading_output;
  } else if constexpr (preset_type == PresetType::input) {
    model = self->autoloading_output_model;

    listview = self->listview_autoloading_input;
  }

  const auto profiles = self->application->presets_manager->get_autoload_profiles(preset_type);

  for (const auto& json : profiles) {
    const auto device = json.value("device", "");
    const auto device_profile = json.value("device-profile", "");
    const auto preset_name = json.value("preset-name", "");

    g_list_store_append(model, ui::holders::create(device, device_profile, preset_name));
  }

  auto* selection = gtk_no_selection_new(G_LIST_MODEL(model));

  gtk_list_view_set_model(listview, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);
}

void setup(PipeManagerBox* self, app::Application* application) {
  self->application = application;

  auto pm = application->pm;

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

  setup_listview_modules(self);
  setup_listview_clients(self);

  setup_listview_autoloading<PresetType::input>(self);
  setup_listview_autoloading<PresetType::output>(self);

  // signals related to device insertion/removal

  self->connections.push_back(pm->sink_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->output_devices_model), n))
              ->id == info.id) {
        return;
      }
    }

    g_list_store_append(self->output_devices_model, ui::holders::create(info));
  }));

  self->connections.push_back(pm->sink_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->output_devices_model), n))
              ->id == info.id) {
        g_list_store_remove(self->output_devices_model, n);

        return;
      }
    }
  }));

  self->connections.push_back(pm->source_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n))
              ->id == info.id) {
        return;
      }
    }

    g_list_store_append(self->input_devices_model, ui::holders::create(info));
  }));

  self->connections.push_back(pm->source_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n))
              ->id == info.id) {
        g_list_store_remove(self->input_devices_model, n);

        return;
      }
    }
  }));

  // signals related to presets creation/destruction

  self->connections.push_back(
      application->presets_manager->user_output_preset_created.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning(log_tag + "can't retrieve information about the preset file"s);

          return;
        }

        for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_presets_string_list)); n++) {
          if (gtk_string_list_get_string(self->output_presets_string_list, n) == preset_name) {
            return;
          }
        }

        gtk_string_list_append(self->output_presets_string_list, preset_name.c_str());
      }));

  self->connections.push_back(
      application->presets_manager->user_output_preset_removed.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning(log_tag + "can't retrieve information about the preset file"s);

          return;
        }

        for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_presets_string_list)); n++) {
          if (gtk_string_list_get_string(self->output_presets_string_list, n) == preset_name) {
            gtk_string_list_remove(self->output_presets_string_list, n);

            return;
          }
        }
      }));

  self->connections.push_back(
      application->presets_manager->user_input_preset_created.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning(log_tag + "can't retrieve information about the preset file"s);

          return;
        }

        for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_presets_string_list)); n++) {
          if (gtk_string_list_get_string(self->input_presets_string_list, n) == preset_name) {
            return;
          }
        }

        gtk_string_list_append(self->input_presets_string_list, preset_name.c_str());
      }));

  self->connections.push_back(
      application->presets_manager->user_input_preset_removed.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning(log_tag + "can't retrieve information about the preset file"s);

          return;
        }

        for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_presets_string_list)); n++) {
          if (gtk_string_list_get_string(self->input_presets_string_list, n) == preset_name) {
            gtk_string_list_remove(self->input_presets_string_list, n);

            return;
          }
        }
      }));

  // signals related to autoload profiles

  self->connections.push_back(application->presets_manager->autoload_input_profiles_changed.connect(
      [=](const std::vector<nlohmann::json>& profiles) {
        std::vector<ui::holders::PresetsAutoloadingHolder*> list;

        for (const auto& json : profiles) {
          const auto device = json.value("device", "");
          const auto device_profile = json.value("device-profile", "");
          const auto preset_name = json.value("preset-name", "");

          list.push_back(ui::holders::create(device, device_profile, preset_name));
        }

        g_list_store_splice(self->autoloading_input_model, 0,
                            g_list_model_get_n_items(G_LIST_MODEL(self->autoloading_input_model)),
                            (gpointer*)(list.data()), list.size());
      }));

  self->connections.push_back(application->presets_manager->autoload_output_profiles_changed.connect(
      [=](const std::vector<nlohmann::json>& profiles) {
        std::vector<ui::holders::PresetsAutoloadingHolder*> list;

        for (const auto& json : profiles) {
          const auto device = json.value("device", "");
          const auto device_profile = json.value("device-profile", "");
          const auto preset_name = json.value("preset-name", "");

          list.push_back(ui::holders::create(device, device_profile, preset_name));
        }

        g_list_store_splice(self->autoloading_output_model, 0,
                            g_list_model_get_n_items(G_LIST_MODEL(self->autoloading_output_model)),
                            (gpointer*)(list.data()), list.size());
      }));
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

  gtk_widget_class_bind_template_callback(widget_class, on_enable_test_signal);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_channel_left);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_channel_right);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_channel_both);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_signal_sine);
  gtk_widget_class_bind_template_callback(widget_class, on_checkbutton_signal_gaussian);
  gtk_widget_class_bind_template_callback(widget_class, on_stack_visible_child_changed);
}

void pipe_manager_box_init(PipeManagerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->input_presets_string_list = gtk_string_list_new(nullptr);
  self->output_presets_string_list = gtk_string_list_new(nullptr);

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->output_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->modules_model = g_list_store_new(ui::holders::module_info_holder_get_type());
  self->clients_model = g_list_store_new(ui::holders::client_info_holder_get_type());
  self->autoloading_input_model = g_list_store_new(ui::holders::presets_autoloading_holder_get_type());
  self->autoloading_output_model = g_list_store_new(ui::holders::presets_autoloading_holder_get_type());

  self->sie_settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");
  self->soe_settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

  prepare_spinbutton<"Hz">(self->spinbutton_test_signal_frequency);

  g_settings_bind(self->sie_settings, "use-default-input-device", self->use_default_input, "active",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->soe_settings, "use-default-output-device", self->use_default_output, "active",
                  G_SETTINGS_BIND_DEFAULT);

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