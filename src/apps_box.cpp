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

  GSettings* settings;

  PipelineType pipeline_type;

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

        g_object_set_data(G_OBJECT(item), "enable", enable);
        g_object_set_data(G_OBJECT(item), "app_icon", app_icon);
        g_object_set_data(G_OBJECT(item), "app_name", app_name);
        g_object_set_data(G_OBJECT(item), "media_name", media_name);

        gtk_list_item_set_activatable(item, 0);
        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_signal_connect(enable, "state-set",
                         G_CALLBACK(+[](GtkSwitch* btn, gboolean state, AppsBox* self) { util::warning("enable"); }),
                         self);
      }),
      self);

  g_signal_connect(factory, "bind",
                   G_CALLBACK(+[](GtkSignalListItemFactory* self, GtkListItem* item, gpointer user_data) {
                     auto* enable = static_cast<GtkSwitch*>(g_object_get_data(G_OBJECT(item), "enable"));
                     auto* app_icon = static_cast<GtkImage*>(g_object_get_data(G_OBJECT(item), "app_icon"));
                     auto* app_name_label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "app_name"));
                     auto* media_name = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "media_name"));

                     auto* holder = static_cast<ui::holders::NodeInfoHolder*>(gtk_list_item_get_item(item));

                     const auto timestamp = holder->ts;
                     const auto stream_id = holder->id;
                     const auto app_name = holder->name;
                     const auto media_class = holder->media_class;

                     auto application_info_update = [=](const NodeInfo& node_info) {
                       //  g_signal_handler_block();
                     };

                     auto connection_info = holder->info_updated.connect(application_info_update);
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

  g_object_unref(self->all_apps_model);  // do not do this to self->apps_model. It is owned by the listview
  g_object_unref(self->settings);

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

  self->apps_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->all_apps_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  setup_listview(self);
}

auto create() -> AppsBox* {
  return static_cast<AppsBox*>(g_object_new(EE_TYPE_APPS_BOX, nullptr));
}

}  // namespace ui::apps_box