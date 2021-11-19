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

#include "apps_box.hpp"

namespace ui::apps_box {

using namespace std::string_literals;

auto constexpr log_tag = "apps_box: ";

struct _AppsBox {
  GtkBox parent_instance{};

  GtkListView* listview;

  app::Application* application;

  GListStore *apps_model, *all_apps_model;

  bool schedule_signal_idle;

  GSettings *settings, *app_settings;

  PipelineType pipeline_type;

  std::unordered_map<uint, bool> enabled_app_list;

  std::vector<sigc::connection> connections;
};

G_DEFINE_TYPE(AppsBox, apps_box, GTK_TYPE_BOX)

auto app_is_blocklisted(AppsBox* self, const std::string& name) -> bool {
  const auto list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "blocklist"));

  return std::ranges::find(list, name) != list.end();
}

void on_app_added(AppsBox* self, const NodeInfo& node_info) {
  // do not add the same stream twice

  for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->all_apps_model)); n++) {
    auto* holder =
        static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->all_apps_model), n));

    if (holder->ts == node_info.timestamp) {
      return;
    }
  }

  auto* holder = ui::holders::create(node_info);

  g_list_store_append(self->all_apps_model, holder);

  if (g_settings_get_boolean(self->settings, "show-blocklisted-apps") != 0 ||
      !app_is_blocklisted(self, node_info.name)) {
    g_list_store_append(self->apps_model, holder);
  }
}

void on_app_removed(AppsBox* self, const util::time_point ts) {
  for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->all_apps_model)); n++) {
    auto* holder =
        static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->all_apps_model), n));

    if (holder->ts == ts) {
      holder->info_updated.clear();  // Disconnecting all the slots before removing the holder from the model

      g_list_store_remove(self->all_apps_model, n);

      break;
    }
  }

  for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->apps_model)); n++) {
    auto* holder = static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->apps_model), n));

    if (holder->ts == ts) {
      g_list_store_remove(self->apps_model, n);

      break;
    }
  }
}

void on_app_changed(AppsBox* self, const util::time_point ts) {
  for (guint n = 0; n < g_list_model_get_n_items(G_LIST_MODEL(self->apps_model)); n++) {
    auto* holder = static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->apps_model), n));

    if (holder->ts == ts) {
      if (auto node_it = self->application->pm->node_map.find(ts); node_it != self->application->pm->node_map.end()) {
        holder->info_updated.emit(node_it->second);

        return;
      }
    }
  }
}

void connect_stream(AppsBox* self, const uint& id, const std::string& media_class) {
  if (media_class == self->application->pm->media_class_output_stream) {
    self->application->pm->connect_stream_output(id);
  } else if (media_class == self->application->pm->media_class_input_stream) {
    self->application->pm->connect_stream_input(id);
  }
}

void disconnect_stream(AppsBox* self, const uint& id, const std::string& media_class) {
  if (media_class == self->application->pm->media_class_output_stream) {
    self->application->pm->disconnect_stream_output(id);
  } else if (media_class == self->application->pm->media_class_input_stream) {
    self->application->pm->disconnect_stream_input(id);
  }
}

void setup_listview(AppsBox* self) {
  auto* selection = gtk_no_selection_new(G_LIST_MODEL(self->apps_model));

  gtk_list_view_set_model(self->listview, GTK_SELECTION_MODEL(selection));

  g_object_unref(selection);

  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, AppsBox* self) {
        auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/app_info.ui");

        auto* top_box = gtk_builder_get_object(builder, "top_box");
        auto* enable = gtk_builder_get_object(builder, "enable");
        auto* app_icon = gtk_builder_get_object(builder, "app_icon");
        auto* app_name = gtk_builder_get_object(builder, "app_name");
        auto* media_name = gtk_builder_get_object(builder, "media_name");
        auto* volume = gtk_builder_get_object(builder, "volume");

        g_object_set_data(G_OBJECT(item), "enable", enable);
        g_object_set_data(G_OBJECT(item), "app_icon", app_icon);
        g_object_set_data(G_OBJECT(item), "app_name", app_name);
        g_object_set_data(G_OBJECT(item), "media_name", media_name);
        g_object_set_data(G_OBJECT(item), "volume", volume);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        auto handler_id_enable = g_signal_connect(
            enable, "state-set", G_CALLBACK(+[](GtkSwitch* btn, gboolean state, AppsBox* self) {
              if (auto* holder = static_cast<ui::holders::NodeInfoHolder*>(g_object_get_data(G_OBJECT(btn), "holder"));
                  holder != nullptr) {
                const auto stream_id = holder->id;
                const auto media_class = holder->media_class;

                if (!app_is_blocklisted(self, holder->name)) {
                  (state) ? connect_stream(self, stream_id, media_class)
                          : disconnect_stream(self, stream_id, media_class);

                  self->enabled_app_list.insert_or_assign(stream_id, state);
                }
              }
            }),
            self);

        auto handler_id_volume = g_signal_connect(
            volume, "value-changed", G_CALLBACK(+[](GtkRange* scale, AppsBox* self) {
              auto* holder = static_cast<ui::holders::NodeInfoHolder*>(g_object_get_data(G_OBJECT(scale), "holder"));

              if (holder != nullptr) {
                const auto node_it = self->application->pm->node_map.find(holder->ts);

                if (node_it != self->application->pm->node_map.end()) {
                  if (node_it->second.proxy != nullptr) {
                    auto vol = static_cast<float>(gtk_range_get_value(GTK_RANGE(scale))) / 100.0F;

                    if (g_settings_get_boolean(self->app_settings, "use-cubic-volumes") != 0) {
                      vol = vol * vol * vol;
                    }

                    PipeManager::set_node_volume(node_it->second.proxy, node_it->second.n_volume_channels, vol);
                  }
                }
              }
            }),
            self);

        g_object_set_data(G_OBJECT(enable), "handler-id", GUINT_TO_POINTER(handler_id_enable));
        g_object_set_data(G_OBJECT(volume), "handler-id", GUINT_TO_POINTER(handler_id_volume));

        g_object_unref(builder);
      }),
      self);

  g_signal_connect(
      factory, "bind", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, AppsBox* self) {
        auto* enable = static_cast<GtkSwitch*>(g_object_get_data(G_OBJECT(item), "enable"));
        auto* app_icon = static_cast<GtkImage*>(g_object_get_data(G_OBJECT(item), "app_icon"));
        auto* app_name_label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "app_name"));
        auto* media_name = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "media_name"));
        auto* volume = static_cast<GtkScale*>(g_object_get_data(G_OBJECT(item), "volume"));

        auto handler_id_enable = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(enable), "handler-id"));
        auto handler_id_volume = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(volume), "handler-id"));

        auto child_item = gtk_list_item_get_item(item);

        auto* holder = static_cast<ui::holders::NodeInfoHolder*>(child_item);

        g_object_set_data(G_OBJECT(enable), "holder", child_item);
        g_object_set_data(G_OBJECT(volume), "holder", child_item);

        const auto timestamp = holder->ts;
        const auto stream_id = holder->id;
        const auto app_name = holder->name;
        const auto media_class = holder->media_class;

        auto application_info_update = [=](const NodeInfo& node_info) {
          if (node_info.state == PW_NODE_STATE_CREATING) {
            // PW_NODE_STATE_CREATING is useless and does not give any meaningful info, therefore skip it
            return;
          }

          gtk_label_set_text(app_name_label, node_info.name.c_str());
          gtk_label_set_text(media_name, node_info.media_name.c_str());

          // updating the enable switch

          g_signal_handler_block(enable, handler_id_enable);

          const auto is_enabled = self->application->pm->stream_is_connected(node_info.id, node_info.media_class);
          const auto is_blocklisted = app_is_blocklisted(self, node_info.name);

          gtk_widget_set_sensitive(GTK_WIDGET(enable), is_enabled || !is_blocklisted);
          gtk_switch_set_active(enable, is_enabled);

          g_signal_handler_unblock(enable, handler_id_enable);

          // updating the volume scale

          g_signal_handler_block(volume, handler_id_volume);

          gtk_widget_set_sensitive(GTK_WIDGET(volume), true);

          if (g_settings_get_boolean(self->app_settings, "use-cubic-volumes") != 0) {
            gtk_range_set_value(GTK_RANGE(volume), 100.0 * std::cbrt(static_cast<double>(node_info.volume)));
          } else {
            gtk_range_set_value(GTK_RANGE(volume), 100.0 * static_cast<double>(node_info.volume));
          }

          g_signal_handler_unblock(volume, handler_id_volume);
        };

        // A call to holder->info_updated.clear() will be made in the unbind signal
        holder->info_updated.connect(application_info_update);
      }),
      self);

  g_signal_connect(factory, "unbind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* self, GtkListItem* item, gpointer user_data) {
                     auto* enable = static_cast<GtkSwitch*>(g_object_get_data(G_OBJECT(item), "enable"));
                     auto* volume = static_cast<GtkScale*>(g_object_get_data(G_OBJECT(item), "volume"));

                     auto* holder = static_cast<ui::holders::NodeInfoHolder*>(gtk_list_item_get_item(item));

                     holder->info_updated.clear();

                     g_object_set_data(G_OBJECT(enable), "handler-id", nullptr);
                     g_object_set_data(G_OBJECT(enable), "holder", nullptr);
                     g_object_set_data(G_OBJECT(volume), "holder", nullptr);
                   }),
                   self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);
}

void setup(AppsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->application = application;
  self->pipeline_type = pipeline_type;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streaminputs");

      auto* pm = application->sie->pm;

      for (const auto& [ts, node] : pm->node_map) {
        if (node.media_class == pm->media_class_input_stream) {
          on_app_added(self, node);
        }
      }

      self->connections.push_back(
          application->sie->pm->stream_input_added.connect([=](const NodeInfo& info) { on_app_added(self, info); }));

      self->connections.push_back(application->sie->pm->stream_input_removed.connect(
          [=](const util::time_point ts) { on_app_removed(self, ts); }));

      self->connections.push_back(application->sie->pm->stream_input_changed.connect(
          [=](const util::time_point ts) { on_app_changed(self, ts); }));

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new("com.github.wwmm.easyeffects.streamoutputs");

      auto* pm = application->soe->pm;

      for (const auto& [ts, node] : pm->node_map) {
        if (node.media_class == pm->media_class_output_stream) {
          on_app_added(self, node);
        }
      }

      self->connections.push_back(
          pm->stream_output_added.connect([=](const NodeInfo& info) { on_app_added(self, info); }));

      self->connections.push_back(application->soe->pm->stream_output_removed.connect(
          [=](const util::time_point ts) { on_app_removed(self, ts); }));

      self->connections.push_back(application->soe->pm->stream_output_changed.connect(
          [=](const util::time_point ts) { on_app_changed(self, ts); }));

      break;
    }
  }
}

void realize(GtkWidget* widget) {
  auto* self = ui::apps_box::EE_APPS_BOX(widget);

  self->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(apps_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_APPS_BOX(widget);

  self->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(apps_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_APPS_BOX(object);

  for (auto& c : self->connections) {
    c.disconnect();
  }

  self->connections.clear();

  g_object_unref(self->all_apps_model);  // do not do this to self->apps_model. It is owned by the listview
  g_object_unref(self->settings);
  g_object_unref(self->app_settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(apps_box_parent_class)->dispose(object);
}

void apps_box_class_init(AppsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  widget_class->realize = realize;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/apps_box.ui");

  gtk_widget_class_bind_template_child(widget_class, AppsBox, listview);
}

void apps_box_init(AppsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->schedule_signal_idle = false;

  self->app_settings = g_settings_new("com.github.wwmm.easyeffects");

  self->apps_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->all_apps_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  self->enabled_app_list = std::unordered_map<uint, bool>();  // Private gtk structures are weird. We have to do this.

  setup_listview(self);
}

auto create() -> AppsBox* {
  return static_cast<AppsBox*>(g_object_new(EE_TYPE_APPS_BOX, nullptr));
}

}  // namespace ui::apps_box