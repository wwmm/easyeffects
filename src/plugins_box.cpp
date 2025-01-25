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

#include "plugins_box.hpp"
#include <STTypes.h>
#include <adwaita.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glibconfig.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <gtk/gtkdroptarget.h>
#include <gtk/gtkshortcut.h>
#include <gtk/gtkwidgetpaintable.h>
#include <sigc++/connection.h>
#include <algorithm>
#include <map>
#include <ranges>
#include <string>
#include <vector>
#include "application.hpp"
#include "autogain.hpp"
#include "autogain_ui.hpp"
#include "bass_enhancer.hpp"
#include "bass_enhancer_ui.hpp"
#include "bass_loudness.hpp"
#include "bass_loudness_ui.hpp"
#include "compressor.hpp"
#include "compressor_ui.hpp"
#include "convolver.hpp"
#include "convolver_ui.hpp"
#include "crossfeed.hpp"
#include "crossfeed_ui.hpp"
#include "crystalizer.hpp"
#include "crystalizer_ui.hpp"
#include "deepfilternet.hpp"
#include "deepfilternet_ui.hpp"
#include "deesser.hpp"
#include "deesser_ui.hpp"
#include "delay.hpp"
#include "delay_ui.hpp"
#include "echo_canceller.hpp"
#include "echo_canceller_ui.hpp"
#include "effects_base.hpp"
#include "equalizer.hpp"
#include "equalizer_ui.hpp"
#include "exciter.hpp"
#include "exciter_ui.hpp"
#include "expander.hpp"
#include "expander_ui.hpp"
#include "filter.hpp"
#include "filter_ui.hpp"
#include "gate.hpp"
#include "gate_ui.hpp"
#include "level_meter.hpp"
#include "level_meter_ui.hpp"
#include "limiter.hpp"
#include "limiter_ui.hpp"
#include "loudness.hpp"
#include "loudness_ui.hpp"
#include "maximizer.hpp"
#include "maximizer_ui.hpp"
#include "multiband_compressor.hpp"
#include "multiband_compressor_ui.hpp"
#include "multiband_gate.hpp"
#include "multiband_gate_ui.hpp"
#include "pipeline_type.hpp"
#include "pitch.hpp"
#include "pitch_ui.hpp"
#include "plugins_menu.hpp"
#include "reverb.hpp"
#include "reverb_ui.hpp"
#include "rnnoise.hpp"
#include "rnnoise_ui.hpp"
#include "speex.hpp"
#include "speex_ui.hpp"
#include "stereo_tools.hpp"
#include "stereo_tools_ui.hpp"
#include "tags_app.hpp"
#include "tags_plugin_name.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::plugins_box {

using namespace std::string_literals;

struct Data {
 public:
  Data() { this->translated = tags::plugin_name::get_translated(); }

  ~Data() { util::debug("data struct destroyed"); }

  bool schedule_signal_idle = false;

  app::Application* application = nullptr;

  PipelineType pipeline_type{};

  std::string schema_path;

  std::map<std::string, std::string> translated;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _PluginsBox {
  GtkBox parent_instance;

  GtkToggleButton* toggle_plugins_list;

  GtkMenuButton* menubutton_plugins;

  GtkOverlay* plugin_overlay;

  AdwStatusPage* overlay_no_plugins;

  GtkListView* listview;

  GtkStack* stack;

  GtkBox *startpoint_box, *endpoint_box;

  GtkImage *startpoint_icon, *endpoint_icon;

  GtkLabel *startpoint_name, *endpoint_name;

  ui::plugins_menu::PluginsMenu* plugins_menu;

  GSettings* settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(PluginsBox, plugins_box, GTK_TYPE_BOX)

template <PipelineType pipeline_type>
void add_plugins_to_stack(PluginsBox* self) {
  EffectsBase* effects_base = nullptr;

  if constexpr (pipeline_type == PipelineType::input) {
    effects_base = self->data->application->sie;
  } else if constexpr (pipeline_type == PipelineType::output) {
    effects_base = self->data->application->soe;
  }

  // saving the current visible page name for later usage

  const std::string visible_page_name =
      (gtk_stack_get_visible_child_name(self->stack) != nullptr) ? gtk_stack_get_visible_child_name(self->stack) : "";

  // removing all plugins

  for (auto* child = gtk_widget_get_first_child(GTK_WIDGET(self->stack)); child != nullptr;) {
    auto* next_child = gtk_widget_get_next_sibling(child);

    uint serial = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(child), "serial"));

    set_ignore_filter_idle_add(serial, true);

    gtk_stack_remove(self->stack, child);

    child = next_child;
  }

  // Adding to the stack the plugins in the list that are not there yet

  auto plugins_list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

  for (const auto& name : plugins_list) {
    auto path = self->data->schema_path + tags::plugin_name::get_base_name(name) + "/" +
                util::to_string(tags::plugin_name::get_id(name)) + "/";

    path.erase(std::remove(path.begin(), path.end(), '_'), path.end());

    if (name.starts_with(tags::plugin_name::autogain)) {
      auto plugin_ptr = effects_base->get_plugin_instance<AutoGain>(name);

      auto* box = ui::autogain_box::create();

      ui::autogain_box::setup(box, plugin_ptr, path);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::bass_enhancer)) {
      auto plugin_ptr = effects_base->get_plugin_instance<BassEnhancer>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::bass_enhancer_box::create();

        ui::bass_enhancer_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::bass_loudness)) {
      auto plugin_ptr = effects_base->get_plugin_instance<BassLoudness>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::bass_loudness_box::create();

        ui::bass_loudness_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::compressor)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Compressor>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::compressor_box::create();

        ui::compressor_box::setup(plugin_box, plugin_ptr, path, self->data->application->pm);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (name.starts_with(tags::plugin_name::convolver)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Convolver>(name);

      auto* box = ui::convolver_box::create();

      ui::convolver_box::setup(box, plugin_ptr, path, self->data->application);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), name.c_str());
    } else if (name.starts_with(tags::plugin_name::crossfeed)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Crossfeed>(name);

      auto* box = ui::crossfeed_box::create();

      ui::crossfeed_box::setup(box, plugin_ptr, path);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), name.c_str());
    } else if (name.starts_with(tags::plugin_name::crystalizer)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Crystalizer>(name);

      auto* box = ui::crystalizer_box::create();

      ui::crystalizer_box::setup(box, plugin_ptr, path);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::deepfilternet)) {
      auto plugin_ptr = effects_base->get_plugin_instance<DeepFilterNet>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::deepfilternet_box::create();

        ui::deepfilternet_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::deesser)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Deesser>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::deesser_box::create();

        ui::deesser_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::delay)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Delay>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::delay_box::create();

        ui::delay_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (name.starts_with(tags::plugin_name::echo_canceller)) {
      auto plugin_ptr = effects_base->get_plugin_instance<EchoCanceller>(name);

      auto* box = ui::echo_canceller_box::create();

      ui::echo_canceller_box::setup(box, plugin_ptr, path);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::exciter)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Exciter>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::exciter_box::create();

        ui::exciter_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::expander)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Expander>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::expander_box::create();

        ui::expander_box::setup(plugin_box, plugin_ptr, path, self->data->application->pm);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::equalizer)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Equalizer>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::equalizer_box::create();

        ui::equalizer_box::setup(plugin_box, plugin_ptr, path, self->data->application);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::filter)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Filter>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::filter_box::create();

        ui::filter_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::gate)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Gate>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::gate_box::create();

        ui::gate_box::setup(plugin_box, plugin_ptr, path, self->data->application->pm);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::level_meter)) {
      auto plugin_ptr = effects_base->get_plugin_instance<LevelMeter>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::level_meter_box::create();

        ui::level_meter_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::limiter)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Limiter>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::limiter_box::create();

        ui::limiter_box::setup(plugin_box, plugin_ptr, path, self->data->application->pm);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::loudness)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Loudness>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::loudness_box::create();

        ui::loudness_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::maximizer)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Maximizer>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::maximizer_box::create();

        ui::maximizer_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::multiband_compressor)) {
      auto plugin_ptr = effects_base->get_plugin_instance<MultibandCompressor>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::multiband_compressor_box::create();

        ui::multiband_compressor_box::setup(plugin_box, plugin_ptr, path, self->data->application->pm);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::multiband_gate)) {
      auto plugin_ptr = effects_base->get_plugin_instance<MultibandGate>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::multiband_gate_box::create();

        ui::multiband_gate_box::setup(plugin_box, plugin_ptr, path, self->data->application->pm);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (name.starts_with(tags::plugin_name::pitch)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Pitch>(name);

      auto* box = ui::pitch_box::create();

      ui::pitch_box::setup(box, plugin_ptr, path);

      gtk_stack_add_named(self->stack, GTK_WIDGET(box), name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::reverb)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Reverb>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::reverb_box::create();

        ui::reverb_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::rnnoise)) {
      auto plugin_ptr = effects_base->get_plugin_instance<RNNoise>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::rnnoise_box::create();

        ui::rnnoise_box::setup(plugin_box, plugin_ptr, path, self->data->application);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::speex)) {
      auto plugin_ptr = effects_base->get_plugin_instance<Speex>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::speex_box::create();

        ui::speex_box::setup(plugin_box, plugin_ptr, path, self->data->application);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    } else if (GtkWidget* box = nullptr; name.starts_with(tags::plugin_name::stereo_tools)) {
      auto plugin_ptr = effects_base->get_plugin_instance<StereoTools>(name);

      if (plugin_ptr->package_installed) {
        auto* plugin_box = ui::stereo_tools_box::create();

        ui::stereo_tools_box::setup(plugin_box, plugin_ptr, path);

        box = GTK_WIDGET(plugin_box);
      } else {
        box = ui::missing_plugin_box(plugin_ptr->name, plugin_ptr->package);
      }

      gtk_stack_add_named(self->stack, box, name.c_str());
    }
  }

  if (plugins_list.empty()) {
    gtk_widget_set_visible(GTK_WIDGET(self->overlay_no_plugins), 1);

    gtk_widget_set_visible(GTK_WIDGET(self->startpoint_box), 0);

    gtk_widget_set_visible(GTK_WIDGET(self->endpoint_box), 0);
  } else {
    gtk_widget_set_visible(GTK_WIDGET(self->overlay_no_plugins), 0);

    gtk_widget_set_visible(GTK_WIDGET(self->startpoint_box), 1);

    gtk_widget_set_visible(GTK_WIDGET(self->endpoint_box), 1);

    if (std::ranges::find(plugins_list, visible_page_name) != plugins_list.end()) {
      gtk_stack_set_visible_child_name(self->stack, visible_page_name.c_str());
    }
  }
}

void show_adjacent_plugin(PluginsBox* self, const int& increment) {
  const auto plugins_list = util::gchar_array_to_vector(g_settings_get_strv(self->settings, "plugins"));

  if (plugins_list.empty()) {
    return;
  }

  const std::string visible_page_name =
      (gtk_stack_get_visible_child_name(self->stack) != nullptr) ? gtk_stack_get_visible_child_name(self->stack) : "";

  if (visible_page_name.empty()) {
    return;
  }

  auto it = std::ranges::find(plugins_list, visible_page_name);

  if (it == plugins_list.end()) {
    return;
  }

  if (auto adj_it = it + increment; adj_it >= plugins_list.begin() && adj_it < plugins_list.end()) {
    gtk_stack_set_visible_child_name(self->stack, (*adj_it).c_str());
  }
}

void on_prev_plugin(PluginsBox* self, GtkButton* button) {
  show_adjacent_plugin(self, -1);
}

void on_next_plugin(PluginsBox* self, GtkButton* button) {
  show_adjacent_plugin(self, 1);
}

void setup_listview(PluginsBox* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(
      factory, "setup", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsBox* self) {
        auto builder = gtk_builder_new_from_resource(tags::resources::plugin_row_ui);

        auto* top_box = gtk_builder_get_object(builder, "top_box");
        auto* plugin_enabled_icon = gtk_builder_get_object(builder, "plugin_enabled_icon");
        auto* plugin_bypassed_icon = gtk_builder_get_object(builder, "plugin_bypassed_icon");
        auto* remove = gtk_builder_get_object(builder, "remove");
        auto* enable = gtk_builder_get_object(builder, "enable");
        auto* drag_handle = gtk_builder_get_object(builder, "drag_handle");

        g_object_set_data(G_OBJECT(item), "top_box", top_box);
        g_object_set_data(G_OBJECT(item), "plugin_enabled_icon", plugin_enabled_icon);
        g_object_set_data(G_OBJECT(item), "plugin_bypassed_icon", plugin_bypassed_icon);
        g_object_set_data(G_OBJECT(item), "name", gtk_builder_get_object(builder, "name"));
        g_object_set_data(G_OBJECT(item), "remove", remove);
        g_object_set_data(G_OBJECT(item), "enable", enable);
        g_object_set_data(G_OBJECT(item), "drag_handle", drag_handle);

        gtk_list_item_set_child(item, GTK_WIDGET(top_box));

        g_object_unref(builder);

        // showing/hiding icons based on wether the mouse is over the plugin row

        auto* controller = gtk_event_controller_motion_new();

        g_object_set_data(G_OBJECT(controller), "remove", remove);
        g_object_set_data(G_OBJECT(controller), "enable", enable);
        g_object_set_data(G_OBJECT(controller), "drag_handle", drag_handle);

        g_signal_connect(controller, "enter",
                         G_CALLBACK(+[](GtkEventControllerMotion* controller, gdouble x, gdouble y, PluginsBox* self) {
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "remove")), 1.0);
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "enable")), 1.0);
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "drag_handle")),
                                                  1.0);
                         }),
                         self);

        g_signal_connect(controller, "leave", G_CALLBACK(+[](GtkEventControllerMotion* controller, PluginsBox* self) {
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "remove")), 0.0);
                           gtk_widget_set_opacity(GTK_WIDGET(g_object_get_data(G_OBJECT(controller), "enable")), 0.0);
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

                  auto insert_after = iter_src - list.begin() < iter_dst - list.begin();

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

  g_signal_connect(
      factory, "bind", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, PluginsBox* self) {
        auto* top_box = static_cast<GtkBox*>(g_object_get_data(G_OBJECT(item), "top_box"));
        auto* label = static_cast<GtkLabel*>(g_object_get_data(G_OBJECT(item), "name"));
        auto* remove = static_cast<GtkButton*>(g_object_get_data(G_OBJECT(item), "remove"));
        auto* enable = static_cast<GtkToggleButton*>(g_object_get_data(G_OBJECT(item), "enable"));

        auto* child_item = gtk_list_item_get_item(item);

        auto page = GTK_STACK_PAGE(child_item);
        auto page_name = gtk_stack_page_get_name(page);
        const auto base_name = tags::plugin_name::get_base_name(page_name);

        g_object_set_data(G_OBJECT(top_box), "page-name", const_cast<char*>(page_name));
        g_object_set_data(G_OBJECT(remove), "page-name", const_cast<char*>(page_name));

        gtk_label_set_text(label, self->data->translated[base_name].c_str());

        gtk_accessible_update_property(GTK_ACCESSIBLE(remove), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                       (_("Remove") + " "s + self->data->translated[base_name]).c_str(), -1);

        // binding the enable button to the bypass key

        auto gname = base_name;

        gname.erase(std::remove(gname.begin(), gname.end(), '_'), gname.end());

        auto schema_path =
            self->data->schema_path + gname + "/" + util::to_string(tags::plugin_name::get_id(page_name)) + "/";

        auto schema_id = tags::app::id + "."s + gname;

        auto* settings = g_settings_new_with_path(schema_id.c_str(), schema_path.c_str());

        gsettings_bind_widget(settings, "bypass", enable, G_SETTINGS_BIND_INVERT_BOOLEAN);

        g_object_unref(settings);
      }),
      self);

  gtk_list_view_set_factory(self->listview, factory);

  g_object_unref(factory);
}

void setup(PluginsBox* self, app::Application* application, PipelineType pipeline_type) {
  self->data->application = application;
  self->data->pipeline_type = pipeline_type;

  switch (pipeline_type) {
    case PipelineType::input: {
      self->settings = g_settings_new(tags::schema::id_input);

      self->data->schema_path = tags::app::path_stream_inputs;

      add_plugins_to_stack<PipelineType::input>(self);

      self->data->gconnections.push_back(g_signal_connect(
          self->settings, "changed::plugins", G_CALLBACK(+[](GSettings* settings, char* key, PluginsBox* self) {
            add_plugins_to_stack<PipelineType::input>(self);
          }),
          self));

      gtk_image_set_from_icon_name(self->startpoint_icon, "audio-input-microphone-symbolic");
      gtk_image_set_from_icon_name(self->endpoint_icon, "ee-applications-multimedia-symbolic");

      gtk_label_set_text(self->startpoint_name, _("Input Device"));
      gtk_label_set_text(self->endpoint_name, _("Recorders"));

      break;
    }
    case PipelineType::output: {
      self->settings = g_settings_new(tags::schema::id_output);

      self->data->schema_path = tags::app::path_stream_outputs;

      add_plugins_to_stack<PipelineType::output>(self);

      self->data->gconnections.push_back(g_signal_connect(
          self->settings, "changed::plugins", G_CALLBACK(+[](GSettings* settings, char* key, PluginsBox* self) {
            add_plugins_to_stack<PipelineType::output>(self);
          }),
          self));

      gtk_image_set_from_icon_name(self->startpoint_icon, "ee-applications-multimedia-symbolic");
      gtk_image_set_from_icon_name(self->endpoint_icon, "audio-speakers-symbolic");

      gtk_label_set_text(self->startpoint_name, _("Players"));
      gtk_label_set_text(self->endpoint_name, _("Output Device"));

      break;
    }
  }

  gsettings_bind_widget(self->settings, "show-plugins-list", self->toggle_plugins_list);

  ui::plugins_menu::setup(self->plugins_menu, application, pipeline_type);

  setup_listview(self);
}

void realize(GtkWidget* widget) {
  auto* self = EE_PLUGINS_BOX(widget);

  self->data->schedule_signal_idle = true;

  GTK_WIDGET_CLASS(plugins_box_parent_class)->realize(widget);
}

void unroot(GtkWidget* widget) {
  auto* self = EE_PLUGINS_BOX(widget);

  self->data->schedule_signal_idle = false;

  GTK_WIDGET_CLASS(plugins_box_parent_class)->unroot(widget);
}

void dispose(GObject* object) {
  auto* self = EE_PLUGINS_BOX(object);

  // Setting post_messages = false for all plugins now that the window is not visible.

  EffectsBase* effects_base = nullptr;

  if (self->data->pipeline_type == PipelineType::input) {
    effects_base = self->data->application->sie;
  } else if (self->data->pipeline_type == PipelineType::output) {
    effects_base = self->data->application->soe;
  }

  for (auto& plugin : effects_base->get_plugins_map() | std::views::values) {
    plugin->set_post_messages(false);
  }

  // Removing gsettings connections

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  // Trying to avoid that the functions scheduled by the plugins are executed when the widgets have already been
  // disposed

  for (auto* child = gtk_widget_get_first_child(GTK_WIDGET(self->stack)); child != nullptr;) {
    auto* next_child = gtk_widget_get_next_sibling(child);

    uint serial = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(child), "serial"));

    set_ignore_filter_idle_add(serial, true);

    child = next_child;
  }

  util::debug("disposed");

  G_OBJECT_CLASS(plugins_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_PLUGINS_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(plugins_box_parent_class)->finalize(object);
}

void plugins_box_class_init(PluginsBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  widget_class->realize = realize;
  widget_class->unroot = unroot;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::plugins_box_ui);

  gtk_widget_class_bind_template_child(widget_class, PluginsBox, toggle_plugins_list);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, menubutton_plugins);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, plugin_overlay);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, overlay_no_plugins);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, listview);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, stack);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, startpoint_box);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, startpoint_icon);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, startpoint_name);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, endpoint_box);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, endpoint_icon);
  gtk_widget_class_bind_template_child(widget_class, PluginsBox, endpoint_name);

  gtk_widget_class_bind_template_callback(widget_class, on_prev_plugin);
  gtk_widget_class_bind_template_callback(widget_class, on_next_plugin);
}

void plugins_box_init(PluginsBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->data->schedule_signal_idle = false;

  self->plugins_menu = ui::plugins_menu::create();

  gtk_menu_button_set_popover(self->menubutton_plugins, GTK_WIDGET(self->plugins_menu));

  gtk_overlay_set_clip_overlay(self->plugin_overlay, GTK_WIDGET(self->overlay_no_plugins), 1);
}

auto create() -> PluginsBox* {
  return static_cast<PluginsBox*>(g_object_new(EE_TYPE_PLUGINS_BOX, nullptr));
}

}  // namespace ui::plugins_box
