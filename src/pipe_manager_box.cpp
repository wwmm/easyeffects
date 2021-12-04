#include "pipe_manager_box.hpp"

namespace ui::pipe_manager_box {

using namespace std::string_literals;

auto constexpr log_tag = "pipe_manager_box: ";

struct _PipeManagerBox {
  GtkBox parent_instance;

  GtkSwitch *use_default_input, *use_default_output, *enable_test_signal;

  GtkDropDown *dropdown_input_devices, *dropdown_output_devices, *dropdown_autoloading_output_devices,
      *dropdown_autoloading_input_devices, *dropdown_autoloading_output_presets, *dropdown_autoloading_input_presets;

  GtkListView *listview_modules, *listview_clients, *listview_autoloading_output, *listview_autoloading_input;

  GtkButton *autoloading_add_input_profile, *autoloading_add_output_profile;

  GtkLabel *header_version, *library_version, *quantum, *max_quantum, *min_quantum, *server_rate;

  GtkSpinButton* spinbutton_test_signal_frequency;

  GListStore *input_devices_model, *output_devices_model, *modules_model, *clients_model, *autoloading_input_model,
      *autoloading_output_model, *autoloading_input_devices_model, *autoloading_output_devices_model;

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

void on_autoloading_add_input_profile(PipeManagerBox* self, GtkButton* btn) {
  auto* holder = static_cast<ui::holders::NodeInfoHolder*>(
      gtk_drop_down_get_selected_item(self->dropdown_autoloading_input_devices));

  if (holder == nullptr) {
    return;
  }

  std::string device_profile;

  for (const auto& device : self->application->pm->list_devices) {
    if (device.id == holder->device_id) {
      device_profile = device.input_route_name;

      break;
    }
  }

  // first we remove any autoloading profile associated to the target device so that our ui is updated

  for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->autoloading_input_model)); n++) {
    auto item = static_cast<ui::holders::PresetsAutoloadingHolder*>(
        g_list_model_get_item(G_LIST_MODEL(self->autoloading_input_model), n));

    if (holder->name == item->device && device_profile == item->device_profile) {
      self->application->presets_manager->remove_autoload(PresetType::input, item->preset_name, item->device,
                                                          item->device_profile);

      break;
    }
  }

  auto selected_preset = gtk_drop_down_get_selected_item(self->dropdown_autoloading_input_presets);

  auto* preset_name = gtk_string_object_get_string(GTK_STRING_OBJECT(selected_preset));

  self->application->presets_manager->add_autoload(PresetType::input, preset_name, holder->name, device_profile);
}

void on_autoloading_add_output_profile(PipeManagerBox* self, GtkButton* btn) {
  auto* holder = static_cast<ui::holders::NodeInfoHolder*>(
      gtk_drop_down_get_selected_item(self->dropdown_autoloading_output_devices));

  if (holder == nullptr) {
    return;
  }

  std::string device_profile;

  for (const auto& device : self->application->pm->list_devices) {
    if (device.id == holder->device_id) {
      device_profile = device.output_route_name;

      break;
    }
  }

  // first we remove any autoloading profile associated to the target device so that our ui is updated

  for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->autoloading_output_model)); n++) {
    auto item = static_cast<ui::holders::PresetsAutoloadingHolder*>(
        g_list_model_get_item(G_LIST_MODEL(self->autoloading_output_model), n));

    if (holder->name == item->device && device_profile == item->device_profile) {
      self->application->presets_manager->remove_autoload(PresetType::output, item->preset_name, item->device,
                                                          item->device_profile);

      break;
    }
  }

  auto selected_preset = gtk_drop_down_get_selected_item(self->dropdown_autoloading_output_presets);

  auto* preset_name = gtk_string_object_get_string(GTK_STRING_OBJECT(selected_preset));

  self->application->presets_manager->add_autoload(PresetType::output, preset_name, holder->name, device_profile);
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
}

void setup_listview_clients(PipeManagerBox* self) {
  auto* selection = gtk_no_selection_new(G_LIST_MODEL(self->clients_model));

  gtk_list_view_set_model(self->listview_clients, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);
}

template <PresetType preset_type>
void setup_listview_autoloading(PipeManagerBox* self) {
  GListStore* model;
  GtkListView* listview;

  if constexpr (preset_type == PresetType::output) {
    model = self->autoloading_output_model;

    listview = self->listview_autoloading_output;
  } else if constexpr (preset_type == PresetType::input) {
    model = self->autoloading_input_model;

    listview = self->listview_autoloading_input;
  }

  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PipeManagerBox* self) {
        auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/autoload_row.ui");

        auto* top_box = gtk_builder_get_object(builder, "top_box");

        g_object_set_data(G_OBJECT(item), "device", gtk_builder_get_object(builder, "device"));
        g_object_set_data(G_OBJECT(item), "device_profile", gtk_builder_get_object(builder, "device_profile"));
        g_object_set_data(G_OBJECT(item), "preset_name", gtk_builder_get_object(builder, "preset_name"));

        auto* remove = gtk_builder_get_object(builder, "remove");

        g_object_set_data(G_OBJECT(item), "remove", remove);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_object_unref(builder);

        g_signal_connect(remove, "clicked", G_CALLBACK(+[](GtkButton* btn, PipeManagerBox* self) {
                           if (auto* holder = static_cast<ui::holders::PresetsAutoloadingHolder*>(
                                   g_object_get_data(G_OBJECT(btn), "holder"));
                               holder != nullptr) {
                             self->application->presets_manager->remove_autoload(
                                 preset_type, holder->preset_name, holder->device, holder->device_profile);
                           }
                         }),
                         self);
      }),
      self);

  g_signal_connect(
      factory, "bind", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PipeManagerBox* self) {
        auto* device = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "device"));
        auto* device_profile = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "device_profile"));
        auto* preset_name = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "preset_name"));
        auto* remove = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "remove"));

        auto* holder = static_cast<ui::holders::PresetsAutoloadingHolder*>(gtk_list_item_get_item(item));

        g_object_set_data(G_OBJECT(remove), "holder", holder);

        gtk_label_set_text(device, holder->device.c_str());
        gtk_label_set_text(device_profile, holder->device_profile.c_str());
        gtk_label_set_text(preset_name, holder->preset_name.c_str());

        gtk_accessible_update_property(GTK_ACCESSIBLE(remove), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                       (_("Remove Autoloading Preset") + " "s + holder->preset_name).c_str(), -1);
      }),
      self);

  gtk_list_view_set_factory(listview, factory);

  g_object_unref(factory);

  // setting the model

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

template <PresetType preset_type>
void setup_dropdown_presets(PipeManagerBox* self) {
  GtkDropDown* dropdown;
  GtkStringList* string_list;

  if constexpr (preset_type == PresetType::output) {
    dropdown = self->dropdown_autoloading_output_presets;

    string_list = self->output_presets_string_list;
  } else if constexpr (preset_type == PresetType::input) {
    dropdown = self->dropdown_autoloading_input_presets;

    string_list = self->input_presets_string_list;
  }

  for (const auto& name : self->application->presets_manager->get_names(preset_type)) {
    gtk_string_list_append(string_list, name.c_str());
  }

  // sorter

  auto* sorter = gtk_string_sorter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* sorter_model = gtk_sort_list_model_new(G_LIST_MODEL(string_list), GTK_SORTER(sorter));

  // setting the dropdown model

  auto* selection = gtk_single_selection_new(G_LIST_MODEL(sorter_model));

  gtk_drop_down_set_model(dropdown, G_LIST_MODEL(selection));

  g_object_unref(selection);
}

void setup_dropdown_devices(PipeManagerBox* self, GtkDropDown* dropdown, GListStore* model) {
  auto* selection = gtk_single_selection_new(G_LIST_MODEL(model));

  gtk_drop_down_set_model(dropdown, G_LIST_MODEL(model));

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
      g_list_store_append(self->autoloading_output_devices_model, ui::holders::create(node));
    } else if (node.media_class == pm->media_class_source || node.media_class == pm->media_class_virtual_source) {
      g_list_store_append(self->input_devices_model, ui::holders::create(node));
      g_list_store_append(self->autoloading_input_devices_model, ui::holders::create(node));
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

  setup_dropdown_devices(self, self->dropdown_input_devices, self->input_devices_model);
  setup_dropdown_devices(self, self->dropdown_output_devices, self->output_devices_model);

  setup_dropdown_devices(self, self->dropdown_autoloading_input_devices, self->autoloading_input_devices_model);
  setup_dropdown_devices(self, self->dropdown_autoloading_output_devices, self->autoloading_output_devices_model);

  setup_dropdown_presets<PresetType::input>(self);
  setup_dropdown_presets<PresetType::output>(self);

  /*
    This connection is being done here after initializing the dropdowns to avoid unnecessary emittions while EE is
    initializing
  */

  g_signal_connect(self->dropdown_input_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, PipeManagerBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->sie_settings, "input-device", holder->name.c_str());
                     }
                   }),
                   self);

  g_signal_connect(self->dropdown_output_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, PipeManagerBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->soe_settings, "output-device", holder->name.c_str());
                     }
                   }),
                   self);

  // initializing the custom device selection dropdowns to the previoulsly used device

  {
    auto* holder_selected =
        static_cast<ui::holders::NodeInfoHolder*>(gtk_drop_down_get_selected_item(self->dropdown_input_devices));

    if (holder_selected != nullptr) {
      const auto input_device_name = g_settings_get_string(self->sie_settings, "input-device");

      if (holder_selected->name != input_device_name) {
        for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
          auto item = static_cast<ui::holders::NodeInfoHolder*>(
              g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n));

          if (item->name == input_device_name) {
            gtk_drop_down_set_selected(self->dropdown_input_devices, n);

            break;
          }
        }
      }
    }
  }

  {
    auto* holder_selected =
        static_cast<ui::holders::NodeInfoHolder*>(gtk_drop_down_get_selected_item(self->dropdown_output_devices));

    if (holder_selected != nullptr) {
      const auto output_device_name = g_settings_get_string(self->soe_settings, "output-device");

      if (holder_selected->name != output_device_name) {
        for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_devices_model)); n++) {
          auto item = static_cast<ui::holders::NodeInfoHolder*>(
              g_list_model_get_item(G_LIST_MODEL(self->output_devices_model), n));

          if (item->name == output_device_name) {
            gtk_drop_down_set_selected(self->dropdown_output_devices, n);

            break;
          }
        }
      }
    }
  }

  // signals related to device insertion/removal

  self->connections.push_back(pm->sink_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->output_devices_model), n))
              ->id == info.id) {
        return;
      }
    }

    g_list_store_append(self->output_devices_model, ui::holders::create(info));
    g_list_store_append(self->autoloading_output_devices_model, ui::holders::create(info));
  }));

  self->connections.push_back(pm->sink_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->output_devices_model), n))
              ->id == info.id) {
        g_list_store_remove(self->output_devices_model, n);
        g_list_store_remove(self->autoloading_output_devices_model, n);

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
    g_list_store_append(self->autoloading_input_devices_model, ui::holders::create(info));
  }));

  self->connections.push_back(pm->source_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      if (static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n))
              ->id == info.id) {
        g_list_store_remove(self->input_devices_model, n);
        g_list_store_remove(self->autoloading_input_devices_model, n);

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
  gtk_widget_class_bind_template_callback(widget_class, on_autoloading_add_input_profile);
  gtk_widget_class_bind_template_callback(widget_class, on_autoloading_add_output_profile);
}

void pipe_manager_box_init(PipeManagerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->input_presets_string_list = gtk_string_list_new(nullptr);
  self->output_presets_string_list = gtk_string_list_new(nullptr);

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->output_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->modules_model = g_list_store_new(ui::holders::module_info_holder_get_type());
  self->clients_model = g_list_store_new(ui::holders::client_info_holder_get_type());
  self->autoloading_input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->autoloading_output_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());
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

  g_signal_connect(
      self->use_default_input, "notify::active",
      G_CALLBACK(+[](GtkSwitch* btn, GParamSpec* pspec, PipeManagerBox* self) {
        if (gtk_switch_get_active(btn) != 0) {
          g_settings_set_string(self->sie_settings, "input-device",
                                self->application->pm->default_input_device.name.c_str());

          auto* holder =
              static_cast<ui::holders::NodeInfoHolder*>(gtk_drop_down_get_selected_item(self->dropdown_input_devices));

          if (holder == nullptr) {
            return;
          }

          if (holder->name != self->application->pm->default_input_device.name) {
            for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
              auto item = static_cast<ui::holders::NodeInfoHolder*>(
                  g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n));

              if (item->name == self->application->pm->default_input_device.name) {
                gtk_drop_down_set_selected(self->dropdown_input_devices, n);

                break;
              }
            }
          }
        }
      }),
      self);

  g_signal_connect(
      self->use_default_output, "notify::active",
      G_CALLBACK(+[](GtkSwitch* btn, GParamSpec* pspec, PipeManagerBox* self) {
        if (gtk_switch_get_active(btn) != 0) {
          g_settings_set_string(self->soe_settings, "output-device",
                                self->application->pm->default_output_device.name.c_str());

          auto* holder =
              static_cast<ui::holders::NodeInfoHolder*>(gtk_drop_down_get_selected_item(self->dropdown_output_devices));

          if (holder == nullptr) {
            return;
          }

          if (holder->name != self->application->pm->default_output_device.name) {
            for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_devices_model)); n++) {
              auto item = static_cast<ui::holders::NodeInfoHolder*>(
                  g_list_model_get_item(G_LIST_MODEL(self->output_devices_model), n));

              if (item->name == self->application->pm->default_output_device.name) {
                gtk_drop_down_set_selected(self->dropdown_output_devices, n);

                break;
              }
            }
          }
        }
      }),
      self);
}

auto create() -> PipeManagerBox* {
  return static_cast<PipeManagerBox*>(g_object_new(EE_TYPE_PIPE_MANAGER_BOX, nullptr));
}

}  // namespace ui::pipe_manager_box