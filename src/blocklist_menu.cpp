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

#include "blocklist_menu.hpp"

namespace ui::blocklist_menu {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  app::Application* application;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _BlocklistMenu {
  GtkPopover parent_instance;

  GtkScrolledWindow* scrolled_window;

  GtkListView* listview;

  GtkText* app_name;

  GtkCheckButton* show_blocklisted_apps;

  GtkStringList* string_list;

  GSettings *settings, *app_settings;

  Data* data;
};

G_DEFINE_TYPE(BlocklistMenu, blocklist_menu, GTK_TYPE_POPOVER)

void on_add_to_blocklist(BlocklistMenu* self, GtkButton* button) {
  auto status = util::add_new_blocklist_entry(self->settings, gtk_editable_get_text(GTK_EDITABLE(self->app_name)));

  if (status) {
    gtk_editable_set_text(GTK_EDITABLE(self->app_name), "");
  }
}

void setup_listview(BlocklistMenu* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, BlocklistMenu* self) {
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        auto* label = gtk_label_new(nullptr);
        auto* button = gtk_button_new_from_icon_name("user-trash-symbolic");

        gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
        gtk_widget_set_hexpand(GTK_WIDGET(label), 1);
        gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 100);

        gtk_box_append(GTK_BOX(box), GTK_WIDGET(label));
        gtk_box_append(GTK_BOX(box), GTK_WIDGET(button));

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(box));

        g_object_set_data(G_OBJECT(item), "name", label);
        g_object_set_data(G_OBJECT(item), "remove", button);

        g_signal_connect(
            button, "clicked", G_CALLBACK(+[](GtkButton* btn, BlocklistMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(btn), "string-object"));
                  string_object != nullptr) {
                auto* name = gtk_string_object_get_string(string_object);

                auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "blocklist"));

                list.erase(
                    std::remove_if(list.begin(), list.end(), [=](const auto& app_name) { return app_name == name; }),
                    list.end());

                g_settings_set_strv(self->settings, "blocklist", util::make_gchar_pointer_vector(list).data());
              }
            }),
            self);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, BlocklistMenu* self) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* btn_remove = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "remove"));

                     auto* child_item = gtk_list_item_get_item(item);
                     auto* string_object = GTK_STRING_OBJECT(child_item);

                     g_object_set_data(G_OBJECT(btn_remove), "string-object", string_object);

                     auto* name = gtk_string_object_get_string(GTK_STRING_OBJECT(child_item));

                     gtk_label_set_text(label, name);
                   }),
                   self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);

  for (auto& name : util::gchar_array_to_vector(g_settings_get_strv(self->settings, "blocklist"))) {
    gtk_string_list_append(self->string_list, name.c_str());
  }

  self->data->gconnections.push_back(g_signal_connect(
      self->settings, "changed::blocklist", G_CALLBACK(+[](GSettings* settings, char* key, BlocklistMenu* self) {
        gtk_string_list_splice(self->string_list, 0, g_list_model_get_n_items(G_LIST_MODEL(self->string_list)),
                               g_settings_get_strv(settings, key));
      }),
      self));
}

void setup(BlocklistMenu* self, app::Application* application, PipelineType pipeline_type) {
  self->data->application = application;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new(tags::schema::id_input);

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new(tags::schema::id_output);

      break;
    }
  }

  g_settings_bind(self->settings, "show-blocklisted-apps", self->show_blocklisted_apps, "active",
                  G_SETTINGS_BIND_DEFAULT);

  setup_listview(self);
}

void show(GtkWidget* widget) {
  auto* self = EE_BLOCKLIST_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->scrolled_window, menu_height);

  GTK_WIDGET_CLASS(blocklist_menu_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_BLOCKLIST_MENU(object);

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);
  g_object_unref(self->app_settings);

  util::debug("disposed");

  G_OBJECT_CLASS(blocklist_menu_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_BLOCKLIST_MENU(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(blocklist_menu_parent_class)->finalize(object);
}

void blocklist_menu_class_init(BlocklistMenuClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::blocklist_menu_ui);

  gtk_widget_class_bind_template_child(widget_class, BlocklistMenu, string_list);

  gtk_widget_class_bind_template_child(widget_class, BlocklistMenu, scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, BlocklistMenu, listview);
  gtk_widget_class_bind_template_child(widget_class, BlocklistMenu, app_name);
  gtk_widget_class_bind_template_child(widget_class, BlocklistMenu, show_blocklisted_apps);

  gtk_widget_class_bind_template_callback(widget_class, on_add_to_blocklist);
}

void blocklist_menu_init(BlocklistMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->app_settings = g_settings_new(tags::app::id);

  g_settings_bind(self->app_settings, "autohide-popovers", self, "autohide", G_SETTINGS_BIND_DEFAULT);
}

auto create() -> BlocklistMenu* {
  return static_cast<BlocklistMenu*>(g_object_new(EE_TYPE_BLOCKLIST_MENU, nullptr));
}

}  // namespace ui::blocklist_menu
