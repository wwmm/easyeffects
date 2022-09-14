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

#include "convolver_menu_impulses.hpp"

namespace ui::convolver_menu_impulses {

using namespace std::string_literals;

enum class ImpulseImportState { success, no_regular_file, no_frame, no_stereo };

auto constexpr irs_ext = ".irs";

std::filesystem::path irs_dir = g_get_user_config_dir() + "/easyeffects/irs"s;

struct _ConvolverMenuImpulses {
  GtkBox parent_instance;

  GtkScrolledWindow* scrolled_window;

  GtkListView* listview;

  GtkSpinButton* ir_width;

  GtkSearchEntry* entry_search;

  GtkStringList* string_list;

  GSettings *settings, *app_settings;

  app::Application* application;
};

G_DEFINE_TYPE(ConvolverMenuImpulses, convolver_menu_impulses, GTK_TYPE_POPOVER)

void append_to_string_list(ConvolverMenuImpulses* self, const std::string& irs_filename) {
  ui::append_to_string_list(self->string_list, irs_filename);
}

void remove_from_string_list(ConvolverMenuImpulses* self, const std::string& irs_filename) {
  ui::remove_from_string_list(self->string_list, irs_filename);
}

auto import_irs_file(const std::string& file_path) -> ImpulseImportState {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    util::warning(p.string() + " is not a file!");

    return ImpulseImportState::no_regular_file;
  }

  auto file = SndfileHandle(file_path.c_str());

  if (file.frames() == 0) {
    util::warning("Cannot import the impulse response! The format may be corrupted or unsupported.");
    util::warning(file_path + " loading failed");

    return ImpulseImportState::no_frame;
  }

  if (file.channels() != 2) {
    util::warning("Only stereo impulse files are supported!");
    util::warning(file_path + " loading failed");

    return ImpulseImportState::no_stereo;
  }

  auto out_path = irs_dir / p.filename();

  out_path.replace_extension(irs_ext);

  std::filesystem::copy_file(p, out_path, std::filesystem::copy_options::overwrite_existing);

  util::debug("Irs file successfully imported to: " + out_path.string());

  return ImpulseImportState::success;
}

void notify_import_error(const ImpulseImportState& import_state, ConvolverMenuImpulses* self) {
  std::string descr;

  switch (import_state) {
    case ImpulseImportState::no_regular_file: {
      descr = _("The File Is Not Regular");

      break;
    }
    case ImpulseImportState::no_frame: {
      descr = _("The Impulse File May Be Corrupted or Unsupported");

      break;
    }
    case ImpulseImportState::no_stereo: {
      descr = _("Only Stereo Impulse Files Are Supported");

      break;
    }
    default:
      return;
  }

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  ui::show_simple_message_dialog(GTK_WIDGET(active_window), _("Impulse File Not Imported"), descr);
}

void on_import_irs_clicked(ConvolverMenuImpulses* self, GtkButton* btn) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  auto* dialog = gtk_file_chooser_native_new(_("Import Impulse File"), active_window, GTK_FILE_CHOOSER_ACTION_OPEN,
                                             _("Open"), _("Cancel"));

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_set_name(filter, _("Impulse Response"));
  gtk_file_filter_add_pattern(filter, "*.irs");
  gtk_file_filter_add_pattern(filter, "*.wav");

  gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

  g_signal_connect(dialog, "response",
                   G_CALLBACK(+[](GtkNativeDialog* dialog, int response, ConvolverMenuImpulses* self) {
                     if (response == GTK_RESPONSE_ACCEPT) {
                       auto* chooser = GTK_FILE_CHOOSER(dialog);
                       auto* file = gtk_file_chooser_get_file(chooser);
                       auto* path = g_file_get_path(file);

                       auto import_state = import_irs_file(path);

                       if (import_state != ImpulseImportState::success) {
                         notify_import_error(import_state, self);
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

void remove_irs_file(const std::string& name) {
  const auto irs_file = irs_dir / std::filesystem::path{name + irs_ext};

  if (std::filesystem::exists(irs_file)) {
    std::filesystem::remove(irs_file);

    util::debug("removed irs file: " + irs_file.string());
  }
}

void setup_listview(ConvolverMenuImpulses* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(factory, "setup",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, ConvolverMenuImpulses* self) {
                     auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
                     auto* label = gtk_label_new(nullptr);
                     auto* load = gtk_button_new_with_label(_("Load"));
                     auto* remove = gtk_button_new_from_icon_name("user-trash-symbolic");

                     gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                     gtk_widget_set_hexpand(GTK_WIDGET(label), 1);

                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(label));
                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(load));
                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(remove));

                     gtk_list_item_set_activatable(item, 0);
                     gtk_list_item_set_child(item, GTK_WIDGET(box));

                     g_object_set_data(G_OBJECT(item), "name", label);
                     g_object_set_data(G_OBJECT(item), "load", load);
                     g_object_set_data(G_OBJECT(item), "remove", remove);

                     g_signal_connect(load, "clicked", G_CALLBACK(+[](GtkButton* btn, ConvolverMenuImpulses* self) {
                                        if (auto* string_object =
                                                GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(btn), "string-object"));
                                            string_object != nullptr) {
                                          auto* name = gtk_string_object_get_string(string_object);

                                          auto irs_file = irs_dir / std::filesystem::path{name};

                                          irs_file += irs_ext;

                                          g_settings_set_string(self->settings, "kernel-path", irs_file.c_str());
                                        }
                                      }),
                                      self);

                     g_signal_connect(remove, "clicked", G_CALLBACK(+[](GtkButton* btn, ConvolverMenuImpulses* self) {
                                        if (auto* string_object =
                                                GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(btn), "string-object"));
                                            string_object != nullptr) {
                                          auto* name = gtk_string_object_get_string(string_object);

                                          remove_irs_file(name);
                                        }
                                      }),
                                      self);
                   }),
                   self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, ConvolverMenuImpulses* self) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* load = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "load"));
                     auto* remove = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "remove"));

                     auto* child_item = gtk_list_item_get_item(item);
                     auto* string_object = GTK_STRING_OBJECT(child_item);

                     g_object_set_data(G_OBJECT(load), "string-object", string_object);
                     g_object_set_data(G_OBJECT(remove), "string-object", string_object);

                     auto* name = gtk_string_object_get_string(GTK_STRING_OBJECT(child_item));

                     gtk_label_set_text(label, name);

                     gtk_accessible_update_property(GTK_ACCESSIBLE(load), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                                    (_("Load Impulse") + " "s + name).c_str(), -1);

                     gtk_accessible_update_property(GTK_ACCESSIBLE(remove), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                                    (_("Remove Impulse") + " "s + name).c_str(), -1);
                   }),
                   self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);

  for (const auto& name : util::get_files_name(irs_dir, irs_ext)) {
    gtk_string_list_append(self->string_list, name.c_str());
  }

  // filter

  auto* filter = gtk_string_filter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* filter_model = gtk_filter_list_model_new(G_LIST_MODEL(self->string_list), GTK_FILTER(filter));

  gtk_filter_list_model_set_incremental(filter_model, 1);

  g_object_bind_property(self->entry_search, "text", filter, "search", G_BINDING_DEFAULT);

  // sorter

  auto* sorter = gtk_string_sorter_new(gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, nullptr, "string"));

  auto* sorter_model = gtk_sort_list_model_new(G_LIST_MODEL(filter_model), GTK_SORTER(sorter));

  // setting the listview model and factory

  auto* selection = gtk_no_selection_new(G_LIST_MODEL(sorter_model));

  gtk_list_view_set_model(self->listview, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);
}

void setup(ConvolverMenuImpulses* self, const std::string& schema_path, app::Application* application) {
  self->application = application;

  self->settings = g_settings_new_with_path(tags::schema::convolver::id, schema_path.c_str());

  setup_listview(self);
}

void show(GtkWidget* widget) {
  auto* self = EE_CONVOLVER_MENU_IMPULSES(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->scrolled_window, menu_height);

  GTK_WIDGET_CLASS(convolver_menu_impulses_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_CONVOLVER_MENU_IMPULSES(object);

  g_object_unref(self->settings);
  g_object_unref(self->app_settings);

  util::debug("disposed");

  G_OBJECT_CLASS(convolver_menu_impulses_parent_class)->dispose(object);
}

void convolver_menu_impulses_class_init(ConvolverMenuImpulsesClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::convolver_menu_impulses_ui);

  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuImpulses, scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuImpulses, listview);
  gtk_widget_class_bind_template_child(widget_class, ConvolverMenuImpulses, entry_search);

  gtk_widget_class_bind_template_callback(widget_class, on_import_irs_clicked);
}

void convolver_menu_impulses_init(ConvolverMenuImpulses* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->string_list = gtk_string_list_new(nullptr);

  self->app_settings = g_settings_new(tags::app::id);

  g_settings_bind(self->app_settings, "autohide-popovers", self, "autohide", G_SETTINGS_BIND_DEFAULT);
}

auto create() -> ConvolverMenuImpulses* {
  return static_cast<ConvolverMenuImpulses*>(g_object_new(EE_TYPE_CONVOLVER_MENU_IMPULSES, nullptr));
}

}  // namespace ui::convolver_menu_impulses
