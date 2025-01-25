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

#include "plugins_menu.hpp"
#include <STTypes.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <gtk/gtkshortcut.h>
#include <sigc++/connection.h>
#include <algorithm>
#include <array>
#include <map>
#include <ranges>
#include <string>
#include <vector>
#include "application.hpp"
#include "pipeline_type.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "util.hpp"

namespace ui::plugins_menu {

using namespace std::string_literals;

struct Data {
 public:
  Data() { this->translated = tags::plugin_name::get_translated(); }

  ~Data() { util::debug("data struct destroyed"); }

  bool schedule_signal_idle;

  app::Application* application;

  std::map<std::string, std::string> translated;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _PluginsMenu {
  GtkPopover parent_instance;

  GtkScrolledWindow* scrolled_window;

  GtkListView* listview;

  GtkStringList* string_list;

  GSettings *settings, *app_settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(PluginsMenu, plugins_menu, GTK_TYPE_POPOVER)

void add_new_plugin_to_pipeline(GtkButton* btn, PluginsMenu* self) {
  auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(btn), "string-object"));

  if (string_object == nullptr) {
    return;
  }

  auto* translated_name = gtk_string_object_get_string(string_object);

  std::string base_name;

  for (const auto& [key, value] : self->data->translated) {
    if (translated_name == value) {
      base_name = key;

      break;
    }
  }

  if (base_name.empty()) {
    return;
  }

  auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

  std::vector<uint> index_list;

  for (const auto& name : list) {
    if (tags::plugin_name::get_base_name(name) == base_name) {
      index_list.emplace_back(tags::plugin_name::get_id(name));
    }
  }

  auto new_id = (index_list.empty()) ? 0 : std::ranges::max(index_list) + 1;

  auto new_name = base_name + "#" + util::to_string(new_id);

  if (list.empty()) {
    list.push_back(new_name);

    g_settings_set_strv(self->settings, "plugins", util::make_gchar_pointer_vector(list).data());

    return;
  }

  // If the list is not empty and the user is careful protecting
  // their device with a plugin of type limiter at the last position
  // of the filter chain, we follow this behaviour trying to insert
  // the new plugin at the second to last position.

  // To do so, we first check if the new plugin is a limiter or the
  // level meter and place it directly at the last position (those
  // plugins do not need to be placed elsewhere and in most of the
  // cases the user wants them at the bottom of the pipeline).

  static constexpr auto limiters_and_meters =
      std::to_array({tags::plugin_name::limiter, tags::plugin_name::maximizer, tags::plugin_name::level_meter});

  if (std::any_of(limiters_and_meters.begin(), limiters_and_meters.end(),
                  [&](const auto& str) { return new_name.starts_with(str); })) {
    list.push_back(new_name);

    g_settings_set_strv(self->settings, "plugins", util::make_gchar_pointer_vector(list).data());

    return;
  }

  // If the new plugin is not one of the above mentioned, we have to
  // check the last plugin of the pipeline handling various cases.

  static constexpr auto limiters = std::to_array({tags::plugin_name::limiter, tags::plugin_name::maximizer});

  if (std::any_of(limiters.begin(), limiters.end(), [&](const auto& str) { return list.back().starts_with(str); })) {
    // If the last plugin is a limiter, we place the new plugin at
    // the second to last position.

    list.insert(list.cend() - 1U, new_name);
  } else if (list.back().starts_with(tags::plugin_name::level_meter)) {
    // If the last plugin is not a limiter, but a level meter, we still
    // try to place the new plugin before a limiter, if this limiter is in
    // the second to last position.
    // The reason is that we still want to preserve the "limiter protection"
    // in case the last plugins are a limiter followed by a meter.

    // Calculate the position of the second to last element.

    const auto second_to_last_pos = std::max(static_cast<int>(list.size()) - 2, 0);

    // Check if the plugin at the second to last position is a limiter.

    if (std::any_of(limiters.begin(), limiters.end(),
                    [&](const auto& str) { return list.at(second_to_last_pos).starts_with(str); })) {
      // It's a limiter, so place the new plugin before it.

      const auto second_to_last_iter = std::max(list.cend() - 2U, list.cbegin());

      list.insert(second_to_last_iter, new_name);
    } else {
      // It's not a limiter, so place the new plugin after the level meter.

      list.push_back(new_name);
    }
  } else {
    // If the last plugin is neither a limiter nor a meter, just place
    // the new plugin at the last position.

    list.push_back(new_name);
  }

  g_settings_set_strv(self->settings, "plugins", util::make_gchar_pointer_vector(list).data());
}

void setup_listview(PluginsMenu* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(factory, "setup",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsMenu* self) {
                     auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
                     auto* label = gtk_label_new(nullptr);
                     auto* button = gtk_button_new_from_icon_name("list-add-symbolic");

                     gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
                     gtk_widget_set_hexpand(GTK_WIDGET(label), 1);

                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(label));
                     gtk_box_append(GTK_BOX(box), GTK_WIDGET(button));

                     gtk_widget_add_css_class(GTK_WIDGET(button), "circular");

                     gtk_list_item_set_activatable(item, 0);
                     gtk_list_item_set_child(item, GTK_WIDGET(box));

                     g_object_set_data(G_OBJECT(item), "name", label);
                     g_object_set_data(G_OBJECT(item), "add", button);

                     g_signal_connect(button, "clicked", G_CALLBACK(add_new_plugin_to_pipeline), self);
                   }),
                   self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsMenu* self) {
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* btn_add = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "add"));

                     auto* child_item = gtk_list_item_get_item(item);
                     auto* string_object = GTK_STRING_OBJECT(child_item);

                     g_object_set_data(G_OBJECT(btn_add), "string-object", string_object);

                     auto* translated_name = gtk_string_object_get_string(GTK_STRING_OBJECT(child_item));

                     gtk_label_set_text(label, translated_name);

                     gtk_accessible_update_property(GTK_ACCESSIBLE(btn_add), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                                    (_("Add") + " "s + translated_name).c_str(), -1);
                   }),
                   self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);

  for (const auto& translated_name : std::views::values(self->data->translated)) {
    gtk_string_list_append(self->string_list, translated_name.c_str());
  }
}

void setup(PluginsMenu* self, app::Application* application, PipelineType pipeline_type) {
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

  setup_listview(self);
}

void show(GtkWidget* widget) {
  auto* self = EE_PLUGINS_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto active_window_height = gtk_widget_get_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->scrolled_window, menu_height);

  GTK_WIDGET_CLASS(plugins_menu_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PLUGINS_MENU(object);

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

  G_OBJECT_CLASS(plugins_menu_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_PLUGINS_MENU(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(plugins_menu_parent_class)->finalize(object);
}

void plugins_menu_class_init(PluginsMenuClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::plugins_menu_ui);

  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, string_list);

  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, listview);
}

void plugins_menu_init(PluginsMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->app_settings = g_settings_new(tags::app::id);

  g_settings_bind(self->app_settings, "autohide-popovers", self, "autohide", G_SETTINGS_BIND_DEFAULT);
}

auto create() -> PluginsMenu* {
  return static_cast<PluginsMenu*>(g_object_new(EE_TYPE_PLUGINS_MENU, nullptr));
}

}  // namespace ui::plugins_menu
