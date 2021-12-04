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

  GtkStack* stack;

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

template <PipelineType pipeline_type>
void add_plugins_to_stack(PluginsBox* self) {
  std::string schema_path;
  EffectsBase* effects_base;

  if constexpr (pipeline_type == PipelineType::input) {
    schema_path = "/com/github/wwmm/easyeffects/streaminputs/";

    effects_base = self->application->sie;
  } else if constexpr (pipeline_type == PipelineType::output) {
    schema_path = "/com/github/wwmm/easyeffects/streamoutputs/";

    effects_base = self->application->soe;
  }

  std::replace(schema_path.begin(), schema_path.end(), '.', '/');

  // saving the current visible page name for later usage

  auto* visible_page_name = gtk_stack_get_visible_child_name(self->stack);

  // removing all plugins

  for (auto child = gtk_widget_get_first_child(GTK_WIDGET(self->stack)); child != nullptr;) {
    auto* next_child = gtk_widget_get_next_sibling(child);

    gtk_stack_remove(self->stack, child);

    child = next_child;
  }

  // Adding to the stack the plugins in the list that are not there yet

  auto plugins_list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

  for (const auto& name : plugins_list) {
    if (name == plugin_name::autogain) {
      auto* box = ui::autogain_box::create();

      ui::autogain_box::setup(box, effects_base->autogain, schema_path + plugin_name::autogain + "/");

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::autogain);
    } else if (name == plugin_name::bass_enhancer) {
      auto* box = ui::bass_enhancer_box::create();

      ui::bass_enhancer_box::setup(box, effects_base->bass_enhancer, schema_path + "bassenhancer/");

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::bass_enhancer);
    } else if (name == plugin_name::compressor) {
      auto* box = ui::compressor_box::create();

      ui::compressor_box::setup(box, effects_base->compressor, schema_path + plugin_name::compressor + "/",
                                self->application->pm);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::compressor);
    } else if (name == plugin_name::convolver) {
      auto* box = ui::convolver_box::create();

      ui::convolver_box::setup(box, effects_base->convolver, schema_path + plugin_name::convolver + "/",
                               self->application);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::convolver);
    } else if (name == plugin_name::crossfeed) {
      auto* box = ui::crossfeed_box::create();

      ui::crossfeed_box::setup(box, effects_base->crossfeed, schema_path + plugin_name::crossfeed + "/");

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::crossfeed);
    } else if (name == plugin_name::crystalizer) {
      auto* box = ui::crystalizer_box::create();

      ui::crystalizer_box::setup(box, effects_base->crystalizer, schema_path + plugin_name::crystalizer + "/");

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::crystalizer);
    } else if (name == plugin_name::exciter) {
      auto* box = ui::exciter_box::create();

      ui::exciter_box::setup(box, effects_base->exciter, schema_path + plugin_name::exciter + "/");

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::exciter);
    } else if (name == plugin_name::equalizer) {
      auto* box = ui::equalizer_box::create();

      ui::equalizer_box::setup(box, effects_base->equalizer, schema_path + plugin_name::equalizer + "/",
                               self->application);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::equalizer);
    } else if (name == plugin_name::limiter) {
      auto* box = ui::limiter_box::create();

      ui::limiter_box::setup(box, effects_base->limiter, schema_path + plugin_name::limiter + "/",
                             self->application->pm);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::limiter);
    } else if (name == plugin_name::maximizer) {
      auto* box = ui::maximizer_box::create();

      ui::maximizer_box::setup(box, effects_base->maximizer, schema_path + plugin_name::maximizer + "/");

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), plugin_name::maximizer);
    }
  }

  if (visible_page_name != nullptr) {
    if (std::ranges::find(plugins_list, visible_page_name) != plugins_list.end()) {
      gtk_stack_set_visible_child_name(self->stack, visible_page_name);
    }
  }
}

void setup_listview(PluginsBox* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsBox* self) {
        auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/plugin_row.ui");

        auto* top_box = gtk_builder_get_object(builder, "top_box");
        auto* plugin_icon = gtk_builder_get_object(builder, "plugin_icon");
        auto* remove = gtk_builder_get_object(builder, "remove");
        auto* drag_handle = gtk_builder_get_object(builder, "drag_handle");

        g_object_set_data(G_OBJECT(item), "top_box", top_box);
        g_object_set_data(G_OBJECT(item), "plugin_icon", plugin_icon);
        g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
        g_object_set_data(G_OBJECT(item), "remove", remove);
        g_object_set_data(G_OBJECT(item), "drag_handle", drag_handle);

        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_object_unref(builder);

        // showing/hiding icons based on wether the mouse is over the plugin row

        auto* controller = gtk_event_controller_motion_new();

        g_object_set_data(G_OBJECT(controller), "remove", remove);
        g_object_set_data(G_OBJECT(controller), "drag_handle", drag_handle);

        g_signal_connect(controller, "enter",
                         G_CALLBACK(+[](GtkEventControllerMotion* controller, gdouble x, gdouble y, PluginsBox* self) {
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "remove")), 1.0);
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "drag_handle")),
                                                  1.0);
                         }),
                         self);

        g_signal_connect(controller, "leave", G_CALLBACK(+[](GtkEventControllerMotion* controller, PluginsBox* self) {
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "remove")), 0.0);
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "drag_handle")),
                                                  0.0);
                         }),
                         self);

        gtk_widget_add_controller(GTK_WIDGET(top_box), controller);

        // Configuring row drag and drop

        auto* drag_source = gtk_drag_source_new();

        gtk_drag_source_set_actions(drag_source, GDK_ACTION_MOVE);

        g_object_set_data(G_OBJECT(drag_source), "top_box", top_box);

        g_signal_connect(
            drag_source, "prepare", G_CALLBACK(+[](GtkDragSource* source, double x, double y, PluginsBox* self) {
              auto* top_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(source), "top_box"));

              auto* paintable = gtk_widget_paintable_new(GTK_WIDGET(top_box));

              gtk_drag_source_set_icon(source, paintable, 0, 0);

              if (auto* plugin_name = static_cast<const char*>(g_object_get_data(G_OBJECT(top_box), "page-name"));
                  plugin_name != nullptr) {
                return gdk_content_provider_new_typed(G_TYPE_STRING, plugin_name);
              }

              return gdk_content_provider_new_typed(G_TYPE_STRING, "");
            }),
            self);

        auto* drop_target = gtk_drop_target_new(G_TYPE_STRING, GDK_ACTION_MOVE);

        g_object_set_data(G_OBJECT(drop_target), "top_box", top_box);

        g_signal_connect(
            drop_target, "drop",
            G_CALLBACK(+[](GtkDropTarget* target, const GValue* value, double x, double y, PluginsBox* self) {
              if (!G_VALUE_HOLDS(value, G_TYPE_STRING)) {
                return false;
              }

              auto* top_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(target), "top_box"));

              if (auto* dst = static_cast<const char*>(g_object_get_data(G_OBJECT(top_box), "page-name"));
                  dst != nullptr) {
                auto* src = g_value_get_string(value);

                if (g_strcmp0(src, dst) != 0) {
                  auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

                  auto iter_src = std::ranges::find(list, src);
                  auto iter_dst = std::ranges::find(list, dst);

                  auto insert_after = (iter_src - list.begin() < iter_dst - list.begin()) ? true : false;

                  list.erase(iter_src);

                  iter_dst = std::ranges::find(list, dst);

                  list.insert(((insert_after) ? (iter_dst + 1) : iter_dst), src);

                  g_settings_set_strv(self->settings, "plugins", util::make_gchar_pointer_vector(list).data());

                  return true;
                }

                return false;
              }

              return false;
            }),
            self);

        gtk_widget_add_controller(GTK_WIDGET(drag_handle), GTK_EVENT_CONTROLLER(drag_source));
        gtk_widget_add_controller(GTK_WIDGET(top_box), GTK_EVENT_CONTROLLER(drop_target));

        g_signal_connect(remove, "clicked", G_CALLBACK(+[](GtkButton* btn, PluginsBox* self) {
                           if (auto* name = static_cast<const char*>(g_object_get_data(G_OBJECT(btn), "page-name"));
                               name != nullptr) {
                             auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

                             list.erase(std::remove_if(list.begin(), list.end(),
                                                       [=](const auto& plugin_name) { return plugin_name == name; }),
                                        list.end());

                             g_settings_set_strv(self->settings, "plugins",
                                                 util::make_gchar_pointer_vector(list).data());
                           }
                         }),
                         self);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsBox* self) {
                     auto* top_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(item), "top_box"));
                     auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
                     auto* remove = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "remove"));
                     auto* plugin_icon = static_cast<GtkImage*>(g_object_get_data(G_OBJECT(item), "plugin_icon"));

                     auto* child_item = gtk_list_item_get_item(item);

                     auto page = GTK_STACK_PAGE(child_item);
                     auto name = gtk_stack_page_get_name(page);

                     g_object_set_data(G_OBJECT(top_box), "page-name", const_cast<char*>(name));
                     g_object_set_data(G_OBJECT(remove), "page-name", const_cast<char*>(name));

                     gtk_label_set_text(label, plugin_name::translated[name].c_str());

                     gtk_accessible_update_property(GTK_ACCESSIBLE(remove), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                                    (_("Remove") + " "s + plugin_name::translated[name]).c_str(), -1);

                     const auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

                     if (const auto iter_name = std::ranges::find(list, name);
                         (iter_name == list.begin() && iter_name != list.end() - 2) || iter_name == list.end() - 1) {
                       gtk_image_set_from_icon_name(plugin_icon, "ee-square-symbolic");
                     } else {
                       gtk_image_set_from_icon_name(plugin_icon, "ee-arrow-down-symbolic");
                     }
                   }),
                   self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);

  // setting the listview model

  auto* selection = gtk_stack_get_pages(self->stack);

  gtk_list_view_set_model(self->listview, selection);

  g_object_unref(selection);
}

void setup(PluginsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;
  self->pipeline_type = pipeline_type;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      add_plugins_to_stack<PipelineType::input>(self);

      self->gconnections.push_back(g_signal_connect(self->settings, "changed::plugins",
                                                    G_CALLBACK(+[](GSettings* settings, char* key, PluginsBox* self) {
                                                      add_plugins_to_stack<PipelineType::input>(self);
                                                    }),
                                                    self));

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      add_plugins_to_stack<PipelineType::output>(self);

      self->gconnections.push_back(g_signal_connect(self->settings, "changed::plugins",
                                                    G_CALLBACK(+[](GSettings* settings, char* key, PluginsBox* self) {
                                                      add_plugins_to_stack<PipelineType::output>(self);
                                                    }),
                                                    self));

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