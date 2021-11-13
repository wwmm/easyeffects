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

#include "presets_menu_ui.hpp"

namespace ui::presets_menu {

using namespace std::string_literals;

auto constexpr log_tag = "presets_menu_ui: ";

struct _PresetsMenu {
  GtkPopover parent_instance{};

  AdwViewStack* stack = nullptr;

  GtkScrolledWindow *output_scrolled_window = nullptr, *input_scrolled_window = nullptr;

  GtkListView *output_listview = nullptr, *input_listview = nullptr;

  GtkText *output_name = nullptr, *input_name = nullptr;

  GtkSearchEntry *output_search = nullptr, *input_search = nullptr;

  GtkLabel *last_used_output = nullptr, *last_used_input = nullptr;

  GtkStringList *output_string_list = nullptr, *input_string_list = nullptr;

  GSettings* settings = nullptr;

  app::Application* application = nullptr;

  std::vector<sigc::connection> connections;
};

G_DEFINE_TYPE(PresetsMenu, presets_menu, GTK_TYPE_POPOVER)

void create_preset(PresetsMenu* self, GtkButton* button) {
  const auto* widget_name = gtk_widget_get_name(GTK_WIDGET(button));

  GtkText* preset_name_box = nullptr;
  PresetType preset_type;

  if (g_strcmp0(widget_name, "output_preset") == 0) {
    preset_name_box = self->output_name;

    preset_type = PresetType::output;
  } else if (g_strcmp0(widget_name, "input_preset") == 0) {
    preset_name_box = self->input_name;

    preset_type = PresetType::input;
  }

  auto name = std::string(g_utf8_make_valid(gtk_editable_get_text(GTK_EDITABLE(preset_name_box)), -1));

  if (name.empty()) {
    return;
  }

  gtk_editable_set_text(GTK_EDITABLE(preset_name_box), "");

  // Truncate if longer than 100 characters

  if (name.length() > 100U) {
    name.resize(100U);
  }

  if (name.find_first_of("\\/") != std::string::npos) {
    util::debug(log_tag + " name "s + name + " has illegal file name characters. Aborting preset creation!"s);

    return;
  }

  self->application->presets_manager->add(preset_type, name);
}

void setup_listview(PresetsMenu* self, GtkListView* listview, PresetType preset_type, GtkStringList* string_list) {
  for (const auto& name : self->application->presets_manager->get_names(preset_type)) {
    gtk_string_list_append(string_list, name.c_str());
  }

  // filter

  auto* filter = gtk_string_filter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* filter_model = gtk_filter_list_model_new(G_LIST_MODEL(string_list), GTK_FILTER(filter));

  gtk_filter_list_model_set_incremental(filter_model, 1);

  switch (preset_type) {
    case PresetType::output: {
      g_object_bind_property(self->output_search, "text", filter, "search", G_BINDING_DEFAULT);
      break;
    }
    case PresetType::input: {
      g_object_bind_property(self->input_search, "text", filter, "search", G_BINDING_DEFAULT);
      break;
    }
  }

  // sorter

  auto* sorter = gtk_string_sorter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* sorter_model = gtk_sort_list_model_new(G_LIST_MODEL(filter_model), GTK_SORTER(sorter));

  // setting the listview model and factory

  auto* selection = gtk_no_selection_new(G_LIST_MODEL(sorter_model));

  gtk_list_view_set_model(listview, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);

  auto* factory = gtk_signal_list_item_factory_new();

  switch (preset_type) {
    case PresetType::output: {
      g_object_set_data(G_OBJECT(factory), "preset-type", const_cast<char*>("output"));
      break;
    }
    case PresetType::input: {
      g_object_set_data(G_OBJECT(factory), "preset-type", const_cast<char*>("input"));
      break;
    }
  }

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, gpointer user_data) {
        auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/preset_row.ui");

        auto* top_box = gtk_builder_get_object(builder, "top_box");
        auto* apply = gtk_builder_get_object(builder, "apply");
        auto* save = gtk_builder_get_object(builder, "save");
        auto* remove = gtk_builder_get_object(builder, "remove");

        auto preset_type = g_object_get_data(G_OBJECT(factory), "preset-type");

        g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
        g_object_set_data(G_OBJECT(item), "apply", apply);
        g_object_set_data(G_OBJECT(item), "save", save);
        g_object_set_data(G_OBJECT(item), "remove", remove);
        g_object_set_data(G_OBJECT(apply), "preset-type", preset_type);
        g_object_set_data(G_OBJECT(save), "preset-type", preset_type);
        g_object_set_data(G_OBJECT(remove), "preset-type", preset_type);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_signal_connect(apply, "clicked", G_CALLBACK(+[](GtkButton* button, gpointer user_data) {
                           auto self = static_cast<PresetsMenu*>(user_data);

                           auto* preset_name = static_cast<char*>(g_object_get_data(G_OBJECT(button), "preset-name"));
                           auto preset_type = static_cast<char*>(g_object_get_data(G_OBJECT(button), "preset-type"));

                           if (g_strcmp0(preset_type, "output") == 0) {
                             self->application->presets_manager->load_preset_file(PresetType::output, preset_name);

                             g_settings_set_string(self->settings, "last-used-output-preset", preset_name);
                           } else if (g_strcmp0(preset_type, "input") == 0) {
                             self->application->presets_manager->load_preset_file(PresetType::input, preset_name);

                             g_settings_set_string(self->settings, "last-used-input-preset", preset_name);
                           }
                         }),
                         user_data);

        g_signal_connect(save, "clicked", G_CALLBACK(+[](GtkButton* button, gpointer user_data) {
                           auto self = static_cast<PresetsMenu*>(user_data);

                           auto* preset_name = static_cast<char*>(g_object_get_data(G_OBJECT(button), "preset-name"));
                           auto preset_type = static_cast<char*>(g_object_get_data(G_OBJECT(button), "preset-type"));

                           if (g_strcmp0(preset_type, "output") == 0) {
                             self->application->presets_manager->save_preset_file(PresetType::output, preset_name);
                           } else if (g_strcmp0(preset_type, "input") == 0) {
                             self->application->presets_manager->save_preset_file(PresetType::input, preset_name);
                           }
                         }),
                         user_data);

        g_signal_connect(remove, "clicked", G_CALLBACK(+[](GtkButton* button, gpointer user_data) {
                           auto self = static_cast<PresetsMenu*>(user_data);

                           auto* preset_name = static_cast<char*>(g_object_get_data(G_OBJECT(button), "preset-name"));
                           auto preset_type = static_cast<char*>(g_object_get_data(G_OBJECT(button), "preset-type"));

                           if (g_strcmp0(preset_type, "output") == 0) {
                             self->application->presets_manager->remove(PresetType::output, preset_name);
                           } else if (g_strcmp0(preset_type, "input") == 0) {
                             self->application->presets_manager->remove(PresetType::input, preset_name);
                           }
                         }),
                         user_data);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* self, GtkListItem* item, gpointer user_data) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* apply = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "apply"));
                     auto* save = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "save"));
                     auto* remove = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "remove"));

                     auto* child_item = gtk_list_item_get_item(item);

                     auto* name = gtk_string_object_get_string(GTK_STRING_OBJECT(child_item));

                     gtk_label_set_text(label, name);

                     g_object_set_data(G_OBJECT(apply), "preset-name", const_cast<char*>(name));
                     g_object_set_data(G_OBJECT(save), "preset-name", const_cast<char*>(name));
                     g_object_set_data(G_OBJECT(remove), "preset-name", const_cast<char*>(name));
                   }),
                   self);

  gtk_list_view_set_factory(listview, factory);

  g_object_unref(factory);
}

void reset_menu_button_label(PresetsMenu* self) {
  const auto names_input = self->application->presets_manager->get_names(PresetType::input);
  const auto names_output = self->application->presets_manager->get_names(PresetType::output);

  if (names_input.empty() && names_output.empty()) {
    g_settings_set_string(self->settings, "last-used-output-preset", _("Presets"));
    g_settings_set_string(self->settings, "last-used-input-preset", _("Presets"));

    return;
  }

  for (const auto& name : names_input) {
    if (name == g_settings_get_string(self->settings, "last-used-input-preset")) {
      return;
    }
  }

  for (const auto& name : names_output) {
    if (name == g_settings_get_string(self->settings, "last-used-output-preset")) {
      return;
    }
  }

  g_settings_set_string(self->settings, "last-used-output-preset", _("Presets"));
  g_settings_set_string(self->settings, "last-used-input-preset", _("Presets"));
}

void setup(PresetsMenu* self, app::Application* application) {
  self->application = application;

  setup_listview(self, self->output_listview, PresetType::output, self->output_string_list);
  setup_listview(self, self->input_listview, PresetType::input, self->input_string_list);

  reset_menu_button_label(self);

  self->connections.push_back(
      self->application->presets_manager->user_output_preset_created.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning(log_tag + "can't retrieve information about the preset file"s);

          return;
        }

        for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_string_list)); n++) {
          if (preset_name == gtk_string_list_get_string(self->output_string_list, n)) {
            return;
          }
        }

        gtk_string_list_append(self->output_string_list, preset_name.c_str());
      }));

  self->connections.push_back(
      self->application->presets_manager->user_output_preset_removed.connect([=](const std::string& preset_name) {
        if (preset_name.empty()) {
          util::warning(log_tag + "can't retrieve information about the preset file"s);

          return;
        }

        for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->output_string_list)); n++) {
          if (preset_name == gtk_string_list_get_string(self->output_string_list, n)) {
            gtk_string_list_remove(self->output_string_list, n);

            return;
          }
        }
      }));
}

void show(GtkWidget* widget) {
  auto* self = EE_PRESETS_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->output_scrolled_window, menu_height);
  gtk_scrolled_window_set_max_content_height(self->input_scrolled_window, menu_height);

  GTK_WIDGET_CLASS(presets_menu_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PRESETS_MENU(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed"s);

  G_OBJECT_CLASS(presets_menu_parent_class)->dispose(object);
}

void presets_menu_class_init(PresetsMenuClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/presets_menu.ui");

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, stack);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_listview);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_name);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, output_search);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, last_used_output);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_listview);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_name);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, input_search);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, last_used_input);

  gtk_widget_class_bind_template_callback(widget_class, create_preset);
}

void presets_menu_init(PresetsMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->output_string_list = gtk_string_list_new(nullptr);
  self->input_string_list = gtk_string_list_new(nullptr);

  self->settings = g_settings_new("com.github.wwmm.easyeffects");

  gtk_label_set_text(self->last_used_output, g_settings_get_string(self->settings, "last-used-output-preset"));
  gtk_label_set_text(self->last_used_input, g_settings_get_string(self->settings, "last-used-input-preset"));

  g_signal_connect(self->settings, "changed::last-used-output-preset",
                   G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto self = static_cast<PresetsMenu*>(user_data);

                     gtk_label_set_text(self->last_used_output, g_settings_get_string(settings, key));
                   }),
                   self);

  g_signal_connect(self->settings, "changed::last-used-input-preset",
                   G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto self = static_cast<PresetsMenu*>(user_data);

                     gtk_label_set_text(self->last_used_input, g_settings_get_string(settings, key));
                   }),
                   self);
}

auto create() -> PresetsMenu* {
  return static_cast<PresetsMenu*>(g_object_new(EE_TYPE_PRESETS_MENU, nullptr));
}

}  // namespace ui::presets_menu

PresetsMenuUi::PresetsMenuUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gio::Settings> refSettings,
                             Application* application)
    : Gtk::Popover(cobject),
      settings(std::move(refSettings)),
      app(application),
      output_string_list(Gtk::StringList::create({"initial_value"})),
      input_string_list(Gtk::StringList::create({"initial_value"})) {
  // loading builder widgets

  import_output = builder->get_widget<Gtk::Button>("import_output");

  import_input = builder->get_widget<Gtk::Button>("import_input");

  // signals connection

  stack_model = stack->get_pages();

  import_output->signal_clicked().connect([=, this]() { import_preset(PresetType::output); });

  import_input->signal_clicked().connect([=, this]() { import_preset(PresetType::input); });

  app->presets_manager->user_output_preset_removed.connect([=, this](const std::string& preset_name) {
    if (preset_name.empty()) {
      util::warning(log_tag + "can't retrieve information about the preset file");

      return;
    }

    for (guint n = 0; n < output_string_list->get_n_items(); n++) {
      if (preset_name == output_string_list->get_string(n).raw()) {
        output_string_list->remove(n);

        return;
      }
    }
  });

  app->presets_manager->user_input_preset_created.connect([=, this](const std::string& preset_name) {
    if (preset_name.empty()) {
      util::warning(log_tag + "can't retrieve information about the preset file");

      return;
    }

    for (guint n = 0; n < input_string_list->get_n_items(); n++) {
      if (input_string_list->get_string(n).raw() == preset_name) {
        return;
      }
    }

    input_string_list->append(preset_name);
  });

  app->presets_manager->user_input_preset_removed.connect([=, this](const std::string& preset_name) {
    if (preset_name.empty()) {
      util::warning(log_tag + "can't retrieve information about the preset file");

      return;
    }

    for (guint n = 0; n < input_string_list->get_n_items(); n++) {
      if (input_string_list->get_string(n).raw() == preset_name) {
        input_string_list->remove(n);

        return;
      }
    }
  });
}

PresetsMenuUi::~PresetsMenuUi() {
  for (auto& c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}

auto PresetsMenuUi::create(Application* app) -> PresetsMenuUi* {
  const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/presets_menu.ui");

  const auto settings = Gio::Settings::create("com.github.wwmm.easyeffects");

  return Gtk::Builder::get_widget_derived<PresetsMenuUi>(builder, "PresetsMenuUi", settings, app);
}

void PresetsMenuUi::import_preset(PresetType preset_type) {
  auto* main_window = dynamic_cast<Gtk::Window*>(app->get_active_window());

  auto dialog = Gtk::FileChooserNative::create(_("Import Preset"), *main_window, Gtk::FileChooser::Action::OPEN,
                                               _("Open"), _("Cancel"));

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("Presets"));
  dialog_filter->add_pattern("*.json");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](const auto& response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        app->presets_manager->import(preset_type, dialog->get_file()->get_path());

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}
