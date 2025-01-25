/*
 *  Copyright © 2017-2025 Wellington Wallace
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
#include <STTypes.h>
#include <adwaita.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glibconfig.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <gtk/gtkexpression.h>
#include <gtk/gtkshortcut.h>
#include <gtk/gtkstringfilter.h>
#include <sigc++/connection.h>
#include <regex>
#include <string>
#include <vector>
#include "application.hpp"
#include "preset_type.hpp"
#include "tags_app.hpp"
#include "tags_resources.hpp"
#include "util.hpp"

namespace ui::presets_menu {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  app::Application* application;

  PresetType preset_type;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _PresetsMenu {
  GtkPopover parent_instance;

  AdwViewStack* stack;

  GtkScrolledWindow *scrolled_window_local, *scrolled_window_community;

  GtkBox* community_main_box;

  GtkListView *listview_local, *listview_community;

  AdwStatusPage* status_page_community_list;

  GtkText* new_preset_name;

  GtkLabel *last_loaded_preset_title, *last_loaded_preset_value;

  GtkStringList *presets_list_local, *presets_list_community;

  GtkSearchEntry* search_community;

  GtkStringFilter* filter_string_community;

  GtkButton* refresh_community_list;

  GSettings* settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(PresetsMenu, presets_menu, GTK_TYPE_POPOVER)

auto closure_community_search_filter(PresetsMenu* self, const char* text) -> const char* {
  const std::string preset_path{text};

  static const auto re_preset_name = std::regex(R"([^/]+$)");

  std::smatch name_match;

  std::regex_search(preset_path.cbegin(), preset_path.cend(), name_match, re_preset_name);

  return (name_match.size() == 1U) ? g_strdup(name_match.str(0).c_str()) : "";
}

void create_preset(PresetsMenu* self, GtkButton* button) {
  std::string name = g_utf8_make_valid(gtk_editable_get_text(GTK_EDITABLE(self->new_preset_name)), -1);

  // Reset input field.
  gtk_editable_set_text(GTK_EDITABLE(self->new_preset_name), "");

  // Remove leading and trailing whitespaces.
  util::str_trim(name);

  static const auto json_ext_re = std::regex(R"((?:\.+json)+$)", std::regex::icase);

  // Remove the json extension at the end, if present
  // (it will be added in PresetsManager::add()).
  if (std::smatch sm; std::regex_search(name, sm, json_ext_re)) {
    name = std::regex_replace(name, json_ext_re, "");
  }

  // Check if empty.
  if (name.empty()) {
    return;
  }

  // Truncate if the name is longer than 100 characters.
  if (const auto max_length = 100U; name.size() > max_length) {
    name.resize(max_length);
  }

  // Check for illegal characters.
  if (name.find_first_of("\\/") != std::string::npos) {
    util::warning(" name " + name + " has illegal file name characters. Aborting preset creation!");

    return;
  }

  self->data->application->presets_manager->add(self->data->preset_type, name);
}

void import_preset_from_disk(PresetsMenu* self) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto* dialog = gtk_file_dialog_new();

  gtk_file_dialog_set_title(dialog, _("Import Preset"));
  gtk_file_dialog_set_accept_label(dialog, _("Open"));

  // Open the dialog from the user home folder.
  auto* init_folder = g_file_new_for_path(g_get_home_dir());

  gtk_file_dialog_set_initial_folder(dialog, init_folder);

  g_object_unref(init_folder);

  GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_add_pattern(filter, "*.json");
  gtk_file_filter_set_name(filter, _("Presets"));

  g_list_store_append(filters, filter);

  g_object_unref(filter);

  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

  g_object_unref(filters);

  gtk_file_dialog_open_multiple(
      dialog, active_window, nullptr,
      +[](GObject* source_object, GAsyncResult* result, gpointer user_data) {
        auto* self = static_cast<PresetsMenu*>(user_data);
        auto* dialog = GTK_FILE_DIALOG(source_object);

        auto* files_list = gtk_file_dialog_open_multiple_finish(dialog, result, nullptr);

        if (files_list == nullptr) {
          return;
        }

        for (guint n = 0U; n < g_list_model_get_n_items(files_list); n++) {
          auto* file = static_cast<GFile*>(g_list_model_get_item(files_list, n));
          auto* path = g_file_get_path(file);

          if (self->data->preset_type == PresetType::output) {
            self->data->application->presets_manager->import_from_filesystem(PresetType::output, path);
          } else if (self->data->preset_type == PresetType::input) {
            self->data->application->presets_manager->import_from_filesystem(PresetType::input, path);
          }

          g_free(path);
        }

        g_object_unref(files_list);
      },
      self);
}

template <PresetType preset_type>
void setup_community_presets_listview(PresetsMenu* self,
                                      GtkListView* listview_community,
                                      GtkStringList* presets_list_community) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PresetsMenu* self) {
        auto builder = gtk_builder_new_from_resource(tags::resources::preset_row_community_ui);

        auto* top_box = gtk_builder_get_object(builder, "top_box");

        auto* name = gtk_builder_get_object(builder, "name");
        auto* package = gtk_builder_get_object(builder, "package");

        auto* try_bt = gtk_builder_get_object(builder, "try");
        auto* import = gtk_builder_get_object(builder, "import");

        g_object_set_data(G_OBJECT(item), "name", name);
        g_object_set_data(G_OBJECT(item), "package", package);
        g_object_set_data(G_OBJECT(item), "try", try_bt);
        g_object_set_data(G_OBJECT(item), "import", import);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_signal_connect(try_bt, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
                           auto* string_object_preset =
                               GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object-preset"));
                           auto* label_package =
                               GTK_LABEL(g_object_get_data(G_OBJECT(button), "gtk-label-package-name"));

                           if (string_object_preset == nullptr || label_package == nullptr) {
                             return;
                           }

                           const std::string preset_path = gtk_string_object_get_string(string_object_preset);

                           const std::string preset_package = gtk_label_get_text(label_package);

                           self->data->application->presets_manager->load_community_preset_file(
                               preset_type, preset_path, preset_package);
                         }),
                         self);

        g_signal_connect(import, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
                           auto* string_object_preset =
                               GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object-preset"));
                           auto* label_package =
                               GTK_LABEL(g_object_get_data(G_OBJECT(button), "gtk-label-package-name"));

                           if (string_object_preset == nullptr || label_package == nullptr) {
                             return;
                           }

                           // community presets are indexed by full_path using stem filenames,
                           // so we need to append the json extension.
                           const std::string preset_path = gtk_string_object_get_string(string_object_preset) +
                                                           self->data->application->presets_manager->json_ext;

                           const std::string preset_package = gtk_label_get_text(label_package);

                           self->data->application->presets_manager->import_from_community_package(
                               preset_type, preset_path, preset_package);
                         }),
                         self);

        g_object_unref(builder);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PresetsMenu* self) {
                     auto* name = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* package = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "package"));

                     auto* try_bt = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "try"));
                     auto* import = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "import"));

                     // Get the full path of the community preset item.
                     auto* preset_path = GTK_STRING_OBJECT(gtk_list_item_get_item(item));

                     // Save community preset path in try and import buttons.
                     g_object_set_data(G_OBJECT(try_bt), "string-object-preset", preset_path);
                     g_object_set_data(G_OBJECT(import), "string-object-preset", preset_path);

                     auto* full_cp_path = gtk_string_object_get_string(preset_path);

                     // Extract package name and preset name from the full path.
                     const auto cp_info =
                         self->data->application->presets_manager->get_community_preset_info(preset_type, full_cp_path);

                     // Set labels.
                     gtk_label_set_text(name, cp_info.first.c_str());
                     gtk_label_set_text(package, cp_info.second.c_str());

                     // Save community preset package label in try and import buttons.
                     g_object_set_data(G_OBJECT(try_bt), "gtk-label-package-name", package);
                     g_object_set_data(G_OBJECT(import), "gtk-label-package-name", package);
                   }),
                   self);

  auto refresh_community_listview = +[](GtkButton* button, PresetsMenu* self) {
    // Empty the list, if it's populated.
    if (const auto n_items = g_list_model_get_n_items(G_LIST_MODEL(self->presets_list_community)); n_items != 0U) {
      gtk_string_list_splice(self->presets_list_community, 0U, n_items, nullptr);
    }

    const auto cp_paths = self->data->application->presets_manager->get_all_community_presets_paths(preset_type);

    // If there are no paths, show the AdwStatusPage and exit.
    if (cp_paths.empty()) {
      gtk_widget_set_visible(GTK_WIDGET(self->community_main_box), 0);
      gtk_widget_set_visible(GTK_WIDGET(self->status_page_community_list), 1);

      return;
    }

    // If there are paths, fill the empty list and hide the AdwStatusPage.
    for (const auto& path : cp_paths) {
      gtk_string_list_append(self->presets_list_community, path.c_str());
    }

    gtk_widget_set_visible(GTK_WIDGET(self->status_page_community_list), 0);
    gtk_widget_set_visible(GTK_WIDGET(self->community_main_box), 1);
  };

  g_signal_connect(self->refresh_community_list, "clicked", G_CALLBACK(refresh_community_listview), self);

  gtk_list_view_set_factory(listview_community, factory);

  g_object_unref(factory);

  // Initialize the list
  refresh_community_listview(self->refresh_community_list, self);
}

template <PresetType preset_type>
void setup_local_presets_listview(PresetsMenu* self, GtkListView* listview_local, GtkStringList* presets_list_local) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PresetsMenu* self) {
        auto builder = gtk_builder_new_from_resource(tags::resources::preset_row_ui);

        auto* top_box = gtk_builder_get_object(builder, "top_box");
        auto* apply = gtk_builder_get_object(builder, "apply");
        auto* save = gtk_builder_get_object(builder, "save");
        auto* remove = gtk_builder_get_object(builder, "remove");

        auto* confirmation_box = gtk_builder_get_object(builder, "confirmation_box");
        auto* confirmation_label = gtk_builder_get_object(builder, "confirmation_label");
        auto* confirmation_yes = gtk_builder_get_object(builder, "confirmation_yes");
        auto* confirmation_no = gtk_builder_get_object(builder, "confirmation_no");

        g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
        g_object_set_data(G_OBJECT(item), "apply", apply);
        g_object_set_data(G_OBJECT(item), "confirmation_yes", confirmation_yes);

        g_object_set_data(G_OBJECT(save), "confirmation_box", confirmation_box);
        g_object_set_data(G_OBJECT(save), "confirmation_label", confirmation_label);
        g_object_set_data(G_OBJECT(save), "confirmation_yes", confirmation_yes);

        g_object_set_data(G_OBJECT(remove), "confirmation_box", confirmation_box);
        g_object_set_data(G_OBJECT(remove), "confirmation_label", confirmation_label);
        g_object_set_data(G_OBJECT(remove), "confirmation_yes", confirmation_yes);

        g_object_set_data(G_OBJECT(confirmation_yes), "confirmation_box", confirmation_box);
        g_object_set_data(G_OBJECT(confirmation_yes), "confirmation_label", confirmation_label);

        g_object_set_data(G_OBJECT(confirmation_no), "confirmation_label", confirmation_label);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_signal_connect(
            apply, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object"));
                  string_object != nullptr) {
                auto* preset_name = gtk_string_object_get_string(string_object);

                self->data->application->presets_manager->load_local_preset_file(preset_type, preset_name);
              }
            }),
            self);

        g_signal_connect(
            save, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              auto* confirmation_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(button), "confirmation_box"));

              auto* confirmation_label =
                  static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(button), "confirmation_label"));

              auto* confirmation_yes = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(button), "confirmation_yes"));

              gtk_label_set_text(confirmation_label, _("Save?"));

              gtk_widget_add_css_class(GTK_WIDGET(confirmation_label), "warning");

              gtk_widget_set_visible(GTK_WIDGET(confirmation_box), 1);

              g_object_set_data(G_OBJECT(confirmation_yes), "operation", GUINT_TO_POINTER(0));
            }),
            self);

        g_signal_connect(
            remove, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              auto* confirmation_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(button), "confirmation_box"));

              auto* confirmation_label =
                  static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(button), "confirmation_label"));

              auto* confirmation_yes = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(button), "confirmation_yes"));

              gtk_label_set_text(confirmation_label, _("Delete?"));

              gtk_widget_add_css_class(GTK_WIDGET(confirmation_label), "error");

              gtk_widget_set_visible(GTK_WIDGET(confirmation_box), 1);

              g_object_set_data(G_OBJECT(confirmation_yes), "operation", GUINT_TO_POINTER(1));
            }),
            self);

        g_signal_connect(confirmation_no, "clicked", G_CALLBACK(+[](GtkButton* button, GtkBox* box) {
                           gtk_widget_set_visible(GTK_WIDGET(box), 0);

                           auto* confirmation_label =
                               static_cast<GtkBox*>(g_object_get_data(G_OBJECT(button), "confirmation_label"));

                           gtk_widget_remove_css_class(GTK_WIDGET(confirmation_label), "warning");
                           gtk_widget_remove_css_class(GTK_WIDGET(confirmation_label), "error");
                         }),
                         confirmation_box);

        g_signal_connect(
            confirmation_yes, "clicked", G_CALLBACK(+[](GtkButton* button, PresetsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(button), "string-object"));
                  string_object != nullptr) {
                auto* preset_name = gtk_string_object_get_string(string_object);

                const uint operation = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(button), "operation"));

                auto* confirmation_label =
                    static_cast<GtkBox*>(g_object_get_data(G_OBJECT(button), "confirmation_label"));

                switch (operation) {
                  case 0U: {  // save
                    if constexpr (preset_type == PresetType::output) {
                      self->data->application->presets_manager->save_preset_file(PresetType::output, preset_name);
                    } else if constexpr (preset_type == PresetType::input) {
                      self->data->application->presets_manager->save_preset_file(PresetType::input, preset_name);
                    }

                    gtk_widget_remove_css_class(GTK_WIDGET(confirmation_label), "warning");

                    break;
                  }
                  case 1U: {  // delete
                    if constexpr (preset_type == PresetType::output) {
                      self->data->application->presets_manager->remove(PresetType::output, preset_name);
                    } else if constexpr (preset_type == PresetType::input) {
                      self->data->application->presets_manager->remove(PresetType::input, preset_name);
                    }

                    gtk_widget_remove_css_class(GTK_WIDGET(confirmation_label), "error");

                    break;
                  }
                  default:
                    break;
                }
              }

              auto* confirmation_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(button), "confirmation_box"));

              gtk_widget_set_visible(GTK_WIDGET(confirmation_box), 0);
            }),
            self);

        g_object_unref(builder);
      }),
      self);

  g_signal_connect(
      factory, "bind", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PresetsMenu* self) {
        auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
        auto* apply = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "apply"));
        auto* confirmation_yes = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "confirmation_yes"));

        auto* string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(item));

        g_object_set_data(G_OBJECT(apply), "string-object", string_object);
        g_object_set_data(G_OBJECT(confirmation_yes), "string-object", string_object);

        auto* name = gtk_string_object_get_string(string_object);

        gtk_label_set_text(label, name);
      }),
      self);

  gtk_list_view_set_factory(listview_local, factory);

  g_object_unref(factory);

  for (const auto& name : self->data->application->presets_manager->get_local_presets_name(preset_type)) {
    gtk_string_list_append(presets_list_local, name.c_str());
  }
}

void setup(PresetsMenu* self, app::Application* application, PresetType preset_type) {
  self->data->application = application;
  self->data->preset_type = preset_type;

  auto add_to_list = [=](const std::string& preset_name) {
    if (preset_name.empty()) {
      util::warning("can't retrieve information about the preset file");

      return;
    }

    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->presets_list_local)); n++) {
      if (preset_name == gtk_string_list_get_string(self->presets_list_local, n)) {
        return;
      }
    }

    gtk_string_list_append(self->presets_list_local, preset_name.c_str());
  };

  auto remove_from_list = [=](const std::string& preset_name) {
    if (preset_name.empty()) {
      util::warning("can't retrieve information about the preset file");

      return;
    }

    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->presets_list_local)); n++) {
      if (preset_name == gtk_string_list_get_string(self->presets_list_local, n)) {
        gtk_string_list_remove(self->presets_list_local, n);

        return;
      }
    }
  };

  auto update_last_used_preset_labels = +[](GSettings* settings, const char* key, gpointer user_data) {
    auto* self = static_cast<PresetsMenu*>(user_data);

    const std::string preset_name = util::gsettings_get_string(settings, key);

    if (preset_name.empty()) {
      gtk_widget_set_visible(GTK_WIDGET(self->last_loaded_preset_value), 0);

      gtk_label_set_text(self->last_loaded_preset_value, "");
      gtk_label_set_text(self->last_loaded_preset_title, _("No Preset Loaded"));

      return;
    }

    gtk_widget_set_visible(GTK_WIDGET(self->last_loaded_preset_value), 1);

    gtk_label_set_text(self->last_loaded_preset_value, preset_name.c_str());

    const auto* lcp_key = (self->data->preset_type == PresetType::input) ? "last-loaded-input-community-package"
                                                                         : "last-loaded-output-community-package";

    const std::string community_package = util::gsettings_get_string(settings, lcp_key);

    const std::string preset_title =
        ((community_package.empty()) ? _("Last Local Preset Loaded") : _("Last Community Preset Loaded"));

    gtk_label_set_text(self->last_loaded_preset_title, preset_title.c_str());
  };

  if (preset_type == PresetType::output) {
    setup_community_presets_listview<PresetType::output>(self, self->listview_community, self->presets_list_community);

    setup_local_presets_listview<PresetType::output>(self, self->listview_local, self->presets_list_local);

    self->data->connections.push_back(
        self->data->application->presets_manager->user_output_preset_created.connect(add_to_list));

    self->data->connections.push_back(
        self->data->application->presets_manager->user_output_preset_removed.connect(remove_from_list));

    self->data->gconnections.push_back(g_signal_connect(self->settings, "changed::last-loaded-output-preset",
                                                        G_CALLBACK(update_last_used_preset_labels), self));

    // reset last loaded preset label

    const auto names_output = self->data->application->presets_manager->get_local_presets_name(PresetType::output);

    const std::string preset_name = util::gsettings_get_string(self->settings, "last-loaded-output-preset");

    bool reset_key = true;

    if (!preset_name.empty()) {
      for (const auto& name : names_output) {
        if (name == preset_name) {
          reset_key = false;

          break;
        }
      }
    } else {
      reset_key = false;
    }

    if (reset_key) {
      // reset non-empty key and trigger the changed signal
      g_settings_reset(self->settings, "last-loaded-output-preset");
    } else {
      // no need to reset, just update the labels
      update_last_used_preset_labels(self->settings, "last-loaded-output-preset", self);
    }
  } else if (preset_type == PresetType::input) {
    setup_community_presets_listview<PresetType::input>(self, self->listview_community, self->presets_list_community);

    setup_local_presets_listview<PresetType::input>(self, self->listview_local, self->presets_list_local);

    self->data->connections.push_back(
        self->data->application->presets_manager->user_input_preset_created.connect(add_to_list));

    self->data->connections.push_back(
        self->data->application->presets_manager->user_input_preset_removed.connect(remove_from_list));

    self->data->gconnections.push_back(g_signal_connect(self->settings, "changed::last-loaded-input-preset",
                                                        G_CALLBACK(update_last_used_preset_labels), self));

    // reset last loaded preset label

    const auto names_input = self->data->application->presets_manager->get_local_presets_name(PresetType::input);

    const std::string preset_name = util::gsettings_get_string(self->settings, "last-loaded-input-preset");

    bool reset_key = true;

    if (!preset_name.empty()) {
      for (const auto& name : names_input) {
        if (name == preset_name) {
          reset_key = false;

          break;
        }
      }
    } else {
      reset_key = false;
    }

    if (reset_key) {
      // reset non-empty key and trigger the changed signal
      g_settings_reset(self->settings, "last-loaded-input-preset");
    } else {
      // no need to reset, just update the labels
      update_last_used_preset_labels(self->settings, "last-loaded-input-preset", self);
    }
  }
}

void show(GtkWidget* widget) {
  auto* self = EE_PRESETS_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto active_window_height = gtk_widget_get_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->scrolled_window_local, menu_height);

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

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, presets_list_local);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, presets_list_community);

  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, scrolled_window_local);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, scrolled_window_community);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, new_preset_name);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, search_community);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, filter_string_community);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, listview_local);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, listview_community);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, community_main_box);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, status_page_community_list);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, refresh_community_list);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, last_loaded_preset_title);
  gtk_widget_class_bind_template_child(widget_class, PresetsMenu, last_loaded_preset_value);

  gtk_widget_class_bind_template_callback(widget_class, create_preset);
  gtk_widget_class_bind_template_callback(widget_class, import_preset_from_disk);
  gtk_widget_class_bind_template_callback(widget_class, closure_community_search_filter);
}

void presets_menu_init(PresetsMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->settings = g_settings_new(tags::app::id);
}

auto create() -> PresetsMenu* {
  return static_cast<PresetsMenu*>(g_object_new(EE_TYPE_PRESETS_MENU, nullptr));
}

}  // namespace ui::presets_menu
