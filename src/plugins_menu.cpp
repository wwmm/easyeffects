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

#include "plugins_menu.hpp"

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

G_DEFINE_TYPE(PluginsMenu, plugins_menu, GTK_TYPE_POPOVER)

void setup_listview(PluginsMenu* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsMenu* self) {
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

        g_signal_connect(
            button, "clicked", G_CALLBACK(+[](GtkButton* btn, PluginsMenu* self) {
              if (auto* string_object = GTK_STRING_OBJECT(g_object_get_data(G_OBJECT(btn), "string-object"));
                  string_object != nullptr) {
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

                constexpr auto limiter_plugins =
                    std::to_array({tags::plugin_name::limiter, tags::plugin_name::maximizer});

                if (!list.empty() && std::any_of(limiter_plugins.begin(), limiter_plugins.end(),
                                                 [&](const auto& str) { return list.back().starts_with(str); })) {
                  // If the user is careful protecting his/her device with a plugin of
                  // type limiter at the last position of the filter chain, we follow
                  // this behaviour inserting the new plugin at the second last position

                  list.insert(list.cend() - 1U, new_name);
                } else {
                  list.push_back(new_name);
                }

                g_settings_set_strv(self->settings, "plugins", util::make_gchar_pointer_vector(list).data());
              }
            }),
            self);
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

  auto active_window_height = gtk_widget_get_allocated_height(GTK_WIDGET(active_window));

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
