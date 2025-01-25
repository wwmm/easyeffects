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

#include "expander_ui.hpp"
#include <STTypes.h>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <gio/gio.h>
#include <gio/gliststore.h>
#include <glib-object.h>
#include <glib.h>
#include <glibconfig.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <gtk/gtkdropdown.h>
#include <gtk/gtksingleselection.h>
#include <sigc++/connection.h>
#include <memory>
#include <string>
#include <vector>
#include "expander.hpp"
#include "node_info_holder.hpp"
#include "pipe_manager.hpp"
#include "pipe_objects.hpp"
#include "tags_pipewire.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::expander_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0U;

  std::shared_ptr<Expander> expander;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _ExpanderBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label,
      *plugin_credit;

  GtkLabel *gain_label, *sidechain_label, *curve_label, *envelope_label;

  GtkSpinButton *attack, *release, *release_threshold, *threshold, *knee, *ratio, *makeup, *dry, *wet, *preamp,
      *reactivity, *lookahead, *hpf_freq, *lpf_freq;

  GtkToggleButton *listen, *show_native_ui;

  GtkCheckButton* stereo_split;

  GtkDropDown *expander_mode, *sidechain_type, *sidechain_mode, *sidechain_source, *stereo_split_source, *lpf_mode,
      *hpf_mode, *dropdown_input_devices;

  GListStore* input_devices_model;

  GSettings* settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(ExpanderBox, expander_box, GTK_TYPE_BOX)

void on_reset(ExpanderBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void on_show_native_window(ExpanderBox* self, GtkToggleButton* btn) {
  if (gtk_toggle_button_get_active(btn) != 0) {
    self->data->expander->show_native_ui();
  } else {
    self->data->expander->close_native_ui();
  }
}

auto set_dropdown_sensitive(ExpanderBox* self, const guint selected_id) -> gboolean {
  // Sensitive on External Device selected
  return (selected_id == 1U) ? 1 : 0;
}

void setup_dropdown_input_device(ExpanderBox* self) {
  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->input_devices_model));

  g_signal_connect(self->dropdown_input_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, ExpanderBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->settings, "sidechain-input-device", holder->info->name.c_str());
                     }
                   }),
                   self);

  gtk_drop_down_set_model(self->dropdown_input_devices, G_LIST_MODEL(self->input_devices_model));

  g_object_unref(selection);
}

void setup(ExpanderBox* self, std::shared_ptr<Expander> expander, const std::string& schema_path, PipeManager* pm) {
  self->data->expander = expander;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::expander::id, schema_path.c_str());

  expander->set_post_messages(true);

  setup_dropdown_input_device(self);

  for (const auto& [serial, node] : pm->node_map) {
    if (node.name == tags::pipewire::ee_sink_name || node.name == tags::pipewire::ee_source_name) {
      continue;
    }

    if (node.media_class == tags::pipewire::media_class::source ||
        node.media_class == tags::pipewire::media_class::virtual_source ||
        node.media_role == tags::pipewire::media_role::dsp) {
      auto* holder = ui::holders::create(node);

      g_list_store_append(self->input_devices_model, holder);

      g_object_unref(holder);
    }
  }

  self->data->connections.push_back(expander->input_level.connect([=](const float left, const float right) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                       self->input_level_right_label, left, right);
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(expander->output_level.connect([=](const float left, const float right) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                       self->output_level_right_label, left, right);
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(expander->reduction.connect([=](const float value) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          if (!GTK_IS_LABEL(self->gain_label)) {
            return;
          }

          gtk_label_set_text(self->gain_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(expander->envelope.connect([=](const float value) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          if (!GTK_IS_LABEL(self->envelope_label)) {
            return;
          }

          gtk_label_set_text(self->envelope_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(expander->sidechain.connect([=](const float value) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          if (!GTK_IS_LABEL(self->sidechain_label)) {
            return;
          }

          gtk_label_set_text(self->sidechain_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(expander->curve.connect([=](const float value) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          if (!GTK_IS_LABEL(self->curve_label)) {
            return;
          }

          gtk_label_set_text(self->curve_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(pm->source_added.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      auto* holder =
          static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n));

      if (holder->info->id == info.id) {
        g_object_unref(holder);

        return;
      }

      g_object_unref(holder);
    }

    auto* holder = ui::holders::create(info);

    g_list_store_append(self->input_devices_model, holder);

    g_object_unref(holder);
  }));

  self->data->connections.push_back(pm->source_removed.connect([=](const NodeInfo info) {
    for (guint n = 0U; n < g_list_model_get_n_items(G_LIST_MODEL(self->input_devices_model)); n++) {
      auto* holder =
          static_cast<ui::holders::NodeInfoHolder*>(g_list_model_get_item(G_LIST_MODEL(self->input_devices_model), n));

      if (holder->info->id == info.id) {
        g_list_store_remove(self->input_devices_model, n);

        g_object_unref(holder);

        return;
      }

      g_object_unref(holder);
    }
  }));

  gtk_label_set_text(self->plugin_credit, ui::get_plugin_credit_translated(self->data->expander->package).c_str());

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "dry", gtk_spin_button_get_adjustment(self->dry), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "wet", gtk_spin_button_get_adjustment(self->wet), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "attack", gtk_spin_button_get_adjustment(self->attack), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "knee", gtk_spin_button_get_adjustment(self->knee), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "makeup", gtk_spin_button_get_adjustment(self->makeup), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "ratio", gtk_spin_button_get_adjustment(self->ratio), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "release", gtk_spin_button_get_adjustment(self->release), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "threshold", gtk_spin_button_get_adjustment(self->threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-preamp", gtk_spin_button_get_adjustment(self->preamp), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-reactivity", gtk_spin_button_get_adjustment(self->reactivity), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-lookahead", gtk_spin_button_get_adjustment(self->lookahead), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "release-threshold", gtk_spin_button_get_adjustment(self->release_threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hpf-frequency", gtk_spin_button_get_adjustment(self->hpf_freq), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "lpf-frequency", gtk_spin_button_get_adjustment(self->lpf_freq), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-listen", self->listen, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "stereo-split", self->stereo_split, "active", G_SETTINGS_BIND_DEFAULT);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "mode", self->expander_mode);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "sidechain-type", self->sidechain_type);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "sidechain-mode", self->sidechain_mode);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "sidechain-source", self->sidechain_source);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "stereo-split-source", self->stereo_split_source);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "hpf-mode", self->hpf_mode);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "lpf-mode", self->lpf_mode);

  g_settings_bind(ui::get_global_app_settings(), "show-native-plugin-ui", self->show_native_ui, "visible",
                  G_SETTINGS_BIND_DEFAULT);

  // bind source dropdowns sensitive property to split-stereo gsettings boolean

  g_settings_bind(self->settings, "stereo-split", self->sidechain_source, "sensitive",
                  static_cast<GSettingsBindFlags>(G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_INVERT_BOOLEAN));

  g_settings_bind(self->settings, "stereo-split", self->stereo_split_source, "sensitive", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_EXPANDER_BOX(object);

  self->data->expander->close_native_ui();

  set_ignore_filter_idle_add(self->data->serial, true);

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug("disposed");

  G_OBJECT_CLASS(expander_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_EXPANDER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(expander_box_parent_class)->finalize(object);
}

void expander_box_class_init(ExpanderBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::expander_ui);

  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, output_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, plugin_credit);

  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, gain_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, sidechain_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, curve_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, envelope_label);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, attack);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, release);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, release_threshold);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, knee);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, ratio);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, makeup);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, dry);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, wet);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, preamp);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, reactivity);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, lookahead);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, hpf_freq);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, lpf_freq);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, expander_mode);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, sidechain_type);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, sidechain_mode);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, sidechain_source);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, stereo_split_source);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, stereo_split);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, lpf_mode);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, hpf_mode);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, listen);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, dropdown_input_devices);
  gtk_widget_class_bind_template_child(widget_class, ExpanderBox, show_native_ui);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
  gtk_widget_class_bind_template_callback(widget_class, on_show_native_window);
  gtk_widget_class_bind_template_callback(widget_class, set_dropdown_sensitive);
}

void expander_box_init(ExpanderBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  prepare_spinbuttons<"dB">(self->threshold, self->knee, self->makeup, self->preamp);

  prepare_spinbuttons<"Hz">(self->hpf_freq, self->lpf_freq);

  prepare_spinbuttons<"ms">(self->attack, self->release, self->lookahead, self->reactivity);

  prepare_spinbuttons<"">(self->ratio);

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  // The following spinbuttons can assume -inf
  prepare_spinbuttons<"dB", false>(self->release_threshold, self->dry, self->wet);
}

auto create() -> ExpanderBox* {
  return static_cast<ExpanderBox*>(g_object_new(EE_TYPE_EXPANDER_BOX, nullptr));
}

}  // namespace ui::expander_box
