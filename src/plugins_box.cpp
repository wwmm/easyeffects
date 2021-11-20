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

#include "plugins_box.hpp"

namespace ui::plugins_box {

using namespace std::string_literals;

auto constexpr log_tag = "plugins_box: ";

struct _PluginsBox {
  GtkBox parent_instance;

  GtkMenuButton* menubutton_plugins;

  GtkListView* listview;

  AdwViewStack* stack;

  ui::plugins_menu::PluginsMenu* plugins_menu;

  app::Application* application;

  bool schedule_signal_idle;

  PipelineType pipeline_type;

  GtkStringList* string_list;

  GSettings* settings;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(PluginsBox, plugins_box, GTK_TYPE_BOX)

void setup_listview(PluginsBox* self) {
  if (const auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins")); !list.empty()) {
    for (const auto& name : list) {
      gtk_string_list_append(self->string_list, name.c_str());
    }

    // showing the first plugin in the list by default

    const auto* selected_name = gtk_string_list_get_string(self->string_list, 0);

    for (auto* child = gtk_widget_get_first_child(GTK_WIDGET(self->stack)); child != nullptr;
         child = gtk_widget_get_next_sibling(GTK_WIDGET(self->stack))) {
      if (adw_view_stack_page_get_name(ADW_VIEW_STACK_PAGE(child)) == selected_name) {
        adw_view_stack_set_visible_child(self->stack, child);

        break;
      }
    }
  }

  self->gconnections.push_back(g_signal_connect(
      self->settings, "changed::plugins", G_CALLBACK(+[](GSettings* settings, char* key, PluginsBox* self) {
        if (const auto glist = g_settings_get_strv(settings, key); glist != nullptr) {
          gtk_string_list_splice(self->string_list, 0, g_list_model_get_n_items(G_LIST_MODEL(self->string_list)),
                                 glist);

          const auto list = util::gchar_array_to_vector(glist);

          if (!list.empty()) {
            auto* visible_child = adw_view_stack_get_visible_child(self->stack);

            if (visible_child == nullptr) {
              return;
            }

            auto* visible_page_name = adw_view_stack_page_get_name(ADW_VIEW_STACK_PAGE(visible_child));

            if (std::ranges::find(list, visible_page_name) == list.end()) {
              gtk_selection_model_select_item(gtk_list_view_get_model(self->listview), 0, 1);

              for (auto* child = gtk_widget_get_first_child(GTK_WIDGET(self->stack)); child != nullptr;
                   child = gtk_widget_get_next_sibling(GTK_WIDGET(self->stack))) {
                if (adw_view_stack_page_get_name(ADW_VIEW_STACK_PAGE(child)) == list[0]) {
                  adw_view_stack_set_visible_child(self->stack, child);

                  break;
                }
              }
            } else {
              for (size_t m = 0U; m < list.size(); m++) {
                if (list[m] == visible_page_name) {
                  gtk_selection_model_select_item(gtk_list_view_get_model(self->listview), m, 1);

                  break;
                }
              }
            }
          }
        }
      }),
      self));

  // setting the listview model and factory

  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->string_list));
  // auto* selection = gtk_single_selection_new(G_LIST_MODEL(adw_view_stack_get_pages(self->stack)));

  gtk_list_view_set_model(self->listview, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);

  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);
}

void setup(PluginsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;
  self->pipeline_type = pipeline_type;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      break;
    }
  }

  ui::plugins_menu::setup(self->plugins_menu, application, pipeline_type);

  setup_listview(self);
}

void realize(GtkWidget* widget) {
  auto* self = EE_PLUGINS_BOX(widget);

  self->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(plugins_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_PLUGINS_BOX(widget);

  self->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(plugins_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PLUGINS_BOX(object);

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

  G_OBJECT_CLASS(plugins_box_parent_class)->dispose(object);
}

void plugins_box_class_init(PluginsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->realize = realize;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/plugins_box.ui");

  gtk_widget_class_bind_template_child(widget_class, PluginsBox, menubutton_plugins);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, listview);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, stack);
}

void plugins_box_init(PluginsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->schedule_signal_idle = false;

  self->string_list = gtk_string_list_new(nullptr);

  self->plugins_menu = ui::plugins_menu::create();

  gtk_menu_button_set_popover(self->menubutton_plugins, GTK_WIDGET(self->plugins_menu));
}

auto create() -> PluginsBox* {
  return static_cast<PluginsBox*>(g_object_new(EE_TYPE_PLUGINS_BOX, nullptr));
}

}  // namespace ui::plugins_box