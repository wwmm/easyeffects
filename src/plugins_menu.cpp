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

#include "plugins_menu.hpp"

namespace ui::plugins_menu {

using namespace std::string_literals;

auto constexpr log_tag = "plugins_menu: ";

struct _PluginsMenu {
  GtkPopover parent_instance{};

  GtkScrolledWindow* scrolled_window{};

  GtkListView* listview{};

  GtkSearchEntry* plugins_search;

  GtkStringList* string_list{};

  GSettings* settings{};

  app::Application* application;

  std::vector<sigc::connection> connections{};

  std::vector<gulong> gconnections{};
};

G_DEFINE_TYPE(PluginsMenu, plugins_menu, GTK_TYPE_POPOVER)

void setup_listview(PluginsMenu* self) {}

void setup(PluginsMenu* self, app::Application* application) {
  self->application = application;
}

void show(GtkWidget* widget) {
  auto* self = EE_PLUGINS_MENU(widget);

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

  const int menu_height = static_cast<int>(0.5F * static_cast<float>(active_window_height));

  gtk_scrolled_window_set_max_content_height(self->scrolled_window, menu_height);

  GTK_WIDGET_CLASS(plugins_menu_parent_class)->show(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PLUGINS_MENU(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->connections.clear();
  self->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(plugins_menu_parent_class)->dispose(object);
}

void plugins_menu_class_init(PluginsMenuClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->show = show;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/plugins_menu.ui");

  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, listview);
  gtk_widget_class_bind_template_child(widget_class, PluginsMenu, plugins_search);

  //   gtk_widget_class_bind_template_callback(widget_class, on_show_pluginsed_apps);
  //   gtk_widget_class_bind_template_callback(widget_class, on_add_to_plugins);
}

void plugins_menu_init(PluginsMenu* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->string_list = gtk_string_list_new(nullptr);
}

auto create() -> PluginsMenu* {
  return static_cast<PluginsMenu*>(g_object_new(EE_TYPE_PLUGINS_MENU, nullptr));
}

}  // namespace ui::plugins_menu