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

  g_object_unref(self->apps_model);
  g_object_unref(self->all_apps_model);
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
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, device_state);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, latency_status);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, label_global_output_level_left);
  //   gtk_widget_class_bind_template_child(widget_class, AppsBox, label_global_output_level_right);
}

void apps_box_init(AppsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->schedule_signal_idle = false;

  self->apps_model = g_list_store_new(ui::holders::node_info_holder_get_type());
  self->all_apps_model = g_list_store_new(ui::holders::node_info_holder_get_type());
}

auto create() -> AppsBox* {
  return static_cast<AppsBox*>(g_object_new(EE_TYPE_APPS_BOX, nullptr));
}

}  // namespace ui::apps_box