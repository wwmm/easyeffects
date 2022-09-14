/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "presets_menu.hpp"

namespace ui::presets_menu {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  app::Application* application;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _PresetsMenu {
  GtkPopover parent_instance;

  AdwViewStack* stack;

  GtkScrolledWindow *output_scrolled_window, *input_scrolled_window;

  GtkListView *output_listview, *input_listview;

  GtkText *output_name, *input_name;

  GtkLabel *last_used_output, *last_used_input;

  GtkStringList *output_string_list, *input_string_list;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(PresetsMenu, presets_menu, GTK_TYPE_POPOVER)

template <PresetType preset_type>
void create_preset(PresetsMenu* self, GtkButton* button) {
  GtkText* preset_name_box = nullptr;

  if constexpr (preset_type == PresetType::output) {
    preset_name_box = self->output_name;
  } else if constexpr (preset_type == PresetType::input) {
    preset_name_box = self->input_name;
  }

  auto name = std::string(g_utf8_make_valid(gtk_editable_get_text(GTK_EDITABLE(preset_name_box)), -1));

  if (name.empty()) {
    return;
  }

  gtk_editable_set_text(GTK_EDITABLE(preset_name_box), "");

  // Truncate if longer than 100 characters

  if (name.size() > 100U) {
    name.resize(100U);
  }

  if (name.find_first_of("\\/") != std::string::npos) {
    util::debug(" name " + name + " has illegal file name characters. Aborting preset creation!");

    return;
  }

  self->data->application->presets_manager->add(preset_type, name);
}

void create_output_preset(PresetsMenu* self, GtkButton* button) {
  create_preset<PresetType::output>(self, button);
}

void create_input_preset(PresetsMenu* self, GtkButton* button) {
  create_preset<PresetType::input>(self, button);
}

template <PresetType preset_type>
void import_preset(PresetsMenu* self) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto* dialog = gtk_file_chooser_native_new(_("Import Preset"), active_window, GTK_FILE_CHOOSER_ACTION_OPEN, _("Open"),
                                             _("Cancel"));

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_add_pattern(filter, "*.json");
  gtk_file_filter_set_name(filter, _("Presets"));

  gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

  g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkNativeDialog* dialog, int response, PresetsMenu* self) {
                     if (response == GTK_RESPONSE_ACCEPT) {
                       auto* chooser = GTK_FILE_CHOOSER(dialog);
                       auto* file = gtk_file_chooser_get_file(chooser);
                       auto* path = g_file_get_path(file);

                       if constexpr (preset_type == PresetType::output) {
                         self->data->application->presets_manager->import(PresetType::output, path);
                       } else if constexpr (preset_type == PresetType::input) {
                         self->data->application->presets_manager->import(PresetType::input, path);
                       }

                       g_free(path);

                       g_object_unref(file);
                     }

                     g_object_unref(dialog);
                   }),
                   self);

  gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(dialog), 1);
  gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

void import_output_preset(PresetsMenu* self, GtkButton* button) {
  import_preset<PresetType::output>(self);
}

void import_input_preset(PresetsMenu* self, GtkButton* button) {
  import_preset<PresetType::input>(self);
}

template <PresetType preset_type>
void setup_listview(PresetsMenu* self, GtkListView* listview, GtkStringList* string_list) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PresetsMenu* self) {
        auto builder = gtk_builder_new_from_resource(tags::resources::preset_row_ui);

        auto* top_box = gtk_builder_get_object(builder, "top_box");
        auto* apply = gtk_builder_get_object(builder, "apply");
        auto* save = gtk_builder_get_object(builder, "save");
        auto* remove = gtk_builder_get_object(builder, "remove");

        g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
        g_object_set_data(G_OBJECT(item), "apply", apply);
        g_object_set_data(G_OBJECT(item), "save", save);
        g_object_set_data(G_OBJECT(item), "remove", remove);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_signal_connect(
            apply, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object"));
                  string_object != nullptr) {
                auto* preset_name = gtk_string_object_get_string(string_object);

                if constexpr (preset_type == PresetType::output) {
                  if (self->data->application->presets_manager->load_preset_file(PresetType::output, preset_name)) {
                    g_settings_set_string(self->settings, "last-used-output-preset", preset_name);
                  } else {
                    g_settings_reset(self->settings, "last-used-output-preset");
                  }
                } else if constexpr (preset_type == PresetType::input) {
                  if (self->data->application->presets_manager->load_preset_file(PresetType::input, preset_name)) {
                    g_settings_set_string(self->settings, "last-used-input-preset", preset_name);
                  } else {
                    g_settings_reset(self->settings, "last-used-input-preset");
                  }
                }
              }
            }),
            self);

        g_signal_connect(
            save, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object"));
                  string_object != nullptr) {
                auto* preset_name = gtk_string_object_get_string(string_object);

                if constexpr (preset_type == PresetType::output) {
                  self->data->application->presets_manager->save_preset_file(PresetType::output, preset_name);
                } else if constexpr (preset_type == PresetType::input) {
                  self->data->application->presets_manager->save_preset_file(PresetType::input, preset_name);
                }
              }
            }),
            self);

        g_signal_connect(
            remove, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object"));
                  string_object != nullptr) {
                auto* preset_name = gtk_string_object_get_string(string_object);

                if constexpr (preset_type == PresetType::output) {
                  self->data->application->presets_manager->remove(PresetType::output, preset_name);
                } else if constexpr (preset_type == PresetType::input) {
                  self->data->application->presets_manager->remove(PresetType::input, preset_name);
                }
              }
            }),
            self);

        g_object_unref(builder);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PresetsMenu* self) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* apply = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "apply"));
                     auto* save = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "save"));
                     auto* remove = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "remove"));

                     auto* string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(item));

                     g_object_set_data(G_OBJECT(apply), "string-object", string_object);
                     g_object_set_data(G_OBJECT(save), "string-object", string_object);
                     g_object_set_data(G_OBJECT(remove), "string-object", string_object);

                     auto* name = gtk_string_object_get_string(string_object);

                     gtk_label_set_text(label, name);
                   }),
                   self);

  gtk_list_view_set_factory(listview, factory);

  g_object_unref(factory);

  for (const auto& name : self->data->application->presets_manager->get_names(preset_type)) {
    gtk_string_list_append(string_list, name.c_str());
  }
}

void reset_menu_button_label(PresetsMenu* self) {
  const auto names_input = self->data->application->presets_manager->get_names(PresetType::input);
  const auto names_output = self->data->application->presets_manager->get_names(PresetType::output);

  if (names_input.empty() && names_output.empty()) {
    g_settings_set_string(self->settings, "last-used-output-preset", _("Presets"));
    g_settings_set_string(self->settings, "last-used-input-preset", _("Presets"));

    return;
  }

  for (const auto& name : names_input) {
    if (name == util::gsettings_get_string(self->settings, "last-used-input-preset")) {
      return;
    }
  }

  for (const auto& name : names_output) {
    if (name == util::gsettings_get_string(self->settings, "last-used-output-preset")) {
      return;
    }
  }

  g_settings_set_string(self->settings, "last-used-output-preset", _("Presets"));
  g_settings_set_string(self->settings, "last-used-input-preset", _("Presets"));
}

void setup(PresetsMenu* self, app::Application* application) {
  self->data->application = application;

  setup_listview<PresetType::output>(self, self->output_listview, self->output_string_list);
  setup_listview<PresetType::input>(self, self->input_listview, self->input_string_list);

  reset_menu_button_label(self);

  self->data->connections.push_back(
      self->data->application->presets_manager->user_output_preset_created.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning("can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_string_list)); n++) {
          if (preset_name == gtk_string_list_get_string(self->output_string_list, n)) {
            return;
          }
        }

        gtk_string_list_append(self->output_string_list, preset_name.c_str());
      }));

  self->data->connections.push_back(
      self->data->application->presets_manager->user_output_preset_removed.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning("can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_string_list)); n++) {
          if (preset_name == gtk_string_list_get_string(self->output_string_list, n)) {
            gtk_string_list_remove(self->output_string_list, n);

            return;
          }
        }
      }));

  self->data->connections.push_back(
      self->data->application->presets_manager->user_input_preset_created.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning("can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_string_list)); n++) {
          if (preset_name == gtk_string_list_get_string(self->input_string_list, n)) {
            return;
          }
        }

        gtk_string_list_append(self->input_string_list, preset_name.c_str());
      }));

  self->data->connections.push_back(
      self->data->application->presets_manager->user_input_preset_removed.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning("can't retrieve information about the preset file");

          return;
        }

        for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_string_list)); n++) {
          if (preset_name == gtk_string_list_get_string(self->input_string_list, n)) {
            gtk_string_list_remove(self->input_string_list, n);

            return;
          }
        }
      }));
}

void show(GtkWidget* widget) {
  auto* self = EE_PRESETS_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->output_scrolled_window, menu_height);
  gtk_scrolled_window_set_max_content_height(self->input_scrolled_window, menu_height);

  GTK_WIDGET_CLASS(presets_menu_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PRESETS_MENU(object);

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug("disposed");

  G_OBJECT_CLASS(presets_menu_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_PRESETS_MENU(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(presets_menu_parent_class)->finalize(object);
}

void presets_menu_class_init(PresetsMenuClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::presets_menu_ui);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_string_list);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_string_list);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, stack);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_listview);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_name);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, last_used_output);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_listview);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_name);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, last_used_input);

  gtk_widget_class_bind_template_callback(widget_class, create_output_preset);
  gtk_widget_class_bind_template_callback(widget_class, create_input_preset);
  gtk_widget_class_bind_template_callback(widget_class, import_output_preset);
  gtk_widget_class_bind_template_callback(widget_class, import_input_preset);
}

void presets_menu_init(PresetsMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->settings = g_settings_new(tags::app::id);

  gtk_label_set_text(self->last_used_output,
                     util::gsettings_get_string(self->settings, "last-used-output-preset").c_str());
  gtk_label_set_text(self->last_used_input,
                     util::gsettings_get_string(self->settings, "last-used-input-preset").c_str());

  self->data->gconnections.push_back(
      g_signal_connect(self->settings, "changed::last-used-output-preset",
                       G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                         auto self = static_cast<PresetsMenu*>(user_data);

                         gtk_label_set_text(self->last_used_output, util::gsettings_get_string(settings, key).c_str());
                       }),
                       self));

  self->data->gconnections.push_back(
      g_signal_connect(self->settings, "changed::last-used-input-preset",
                       G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                         auto self = static_cast<PresetsMenu*>(user_data);

                         gtk_label_set_text(self->last_used_input, util::gsettings_get_string(settings, key).c_str());
                       }),
                       self));
}

auto create() -> PresetsMenu* {
  return static_cast<PresetsMenu*>(g_object_new(EE_TYPE_PRESETS_MENU, nullptr));
}

}  // namespace ui::presets_menu
