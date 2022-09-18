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

#include "gate_ui.hpp"

namespace ui::gate_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<Gate> gate;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _GateBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkLabel *attack_zone_start_label, *attack_threshold_label, *release_zone_start_label, *release_threshold_label;

  GtkLabel *gain_label, *sidechain_label, *curve_label, *envelope_label;

  GtkLevelBar* gating;

  GtkLabel* gating_label;

  GtkToggleButton* hysteresis;

  GtkSpinButton *attack, *release, *curve_threshold, *curve_zone, *hysteresis_threshold, *hysteresis_zone, *dry, *wet,
      *reduction, *makeup, *preamp, *reactivity, *lookahead, *hpf_freq, *lpf_freq;

  GtkComboBoxText *sidechain_input, *sidechain_mode, *sidechain_source, *lpf_mode, *hpf_mode;

  GtkToggleButton* listen;

  GtkDropDown* dropdown_input_devices;

  GListStore* input_devices_model;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(GateBox, gate_box, GTK_TYPE_BOX)

void on_reset(GateBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

auto set_dropdown_sensitive(GateBox* self, const char* active_id) -> gboolean {
  if (g_strcmp0(active_id, "External") == 0) {
    return 1;
  }

  return 0;
}

void setup_dropdown_input_device(GateBox* self) {
  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->input_devices_model));

  g_signal_connect(self->dropdown_input_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, GateBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->settings, "sidechain-input-device", holder->info->name.c_str());
                     }
                   }),
                   self);

  gtk_drop_down_set_model(self->dropdown_input_devices, G_LIST_MODEL(self->input_devices_model));

  g_object_unref(selection);
}

void setup(GateBox* self, std::shared_ptr<Gate> gate, const std::string& schema_path, PipeManager* pm) {
  self->data->gate = gate;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::gate::id, schema_path.c_str());

  gate->set_post_messages(true);

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

  self->data->connections.push_back(gate->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(gate->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(gate->attack_zone_start.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->attack_zone_start_label)) {
        return;
      }

      gtk_label_set_text(self->attack_zone_start_label,
                         fmt::format(ui::get_user_locale(), "{0:.1Lf}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->attack_threshold.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->attack_threshold_label)) {
        return;
      }

      gtk_label_set_text(self->attack_threshold_label,
                         fmt::format(ui::get_user_locale(), "{0:.1Lf}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->release_zone_start.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->release_zone_start_label)) {
        return;
      }

      gtk_label_set_text(self->release_zone_start_label,
                         fmt::format(ui::get_user_locale(), "{0:.1Lf}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->release_threshold.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->release_threshold_label)) {
        return;
      }

      gtk_label_set_text(self->release_threshold_label,
                         fmt::format(ui::get_user_locale(), "{0:.1Lf}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->reduction.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->gain_label) || !GTK_IS_LABEL(self->gating_label)) {
        return;
      }

      gtk_label_set_text(self->gating_label, fmt::format("{0:.0Lf}", util::linear_to_db(value)).c_str());
      gtk_label_set_text(self->gain_label, fmt::format("{0:.0Lf}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->gating.connect([=](const double value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LEVEL_BAR(self->gating)) {
        return;
      }

      gtk_level_bar_set_value(self->gating, value);
    });
  }));

  self->data->connections.push_back(gate->envelope.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->envelope_label)) {
        return;
      }

      gtk_label_set_text(self->envelope_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->sidechain.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->sidechain_label)) {
        return;
      }

      gtk_label_set_text(self->sidechain_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(gate->curve.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->curve_label)) {
        return;
      }

      gtk_label_set_text(self->curve_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
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

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "dry", gtk_spin_button_get_adjustment(self->dry), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "wet", gtk_spin_button_get_adjustment(self->wet), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "attack", gtk_spin_button_get_adjustment(self->attack), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "release", gtk_spin_button_get_adjustment(self->release), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "curve-threshold", gtk_spin_button_get_adjustment(self->curve_threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "curve-zone", gtk_spin_button_get_adjustment(self->curve_zone), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hysteresis", self->hysteresis, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hysteresis-threshold", gtk_spin_button_get_adjustment(self->hysteresis_threshold),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hysteresis-zone", gtk_spin_button_get_adjustment(self->hysteresis_zone), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "reduction", gtk_spin_button_get_adjustment(self->reduction), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "makeup", gtk_spin_button_get_adjustment(self->makeup), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-preamp", gtk_spin_button_get_adjustment(self->preamp), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-reactivity", gtk_spin_button_get_adjustment(self->reactivity), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-lookahead", gtk_spin_button_get_adjustment(self->lookahead), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hpf-frequency", gtk_spin_button_get_adjustment(self->hpf_freq), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "lpf-frequency", gtk_spin_button_get_adjustment(self->lpf_freq), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-listen", self->listen, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-input", self->sidechain_input, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-mode", self->sidechain_mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sidechain-source", self->sidechain_source, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hpf-mode", self->hpf_mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "lpf-mode", self->lpf_mode, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_GATE_BOX(object);

  self->data->gate->set_post_messages(false);

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

  G_OBJECT_CLASS(gate_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_GATE_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(gate_box_parent_class)->finalize(object);
}

void gate_box_class_init(GateBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::gate_ui);

  gtk_widget_class_bind_template_child(widget_class, GateBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, GateBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, GateBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, GateBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, GateBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, GateBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, GateBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, output_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, gating);
  gtk_widget_class_bind_template_child(widget_class, GateBox, gating_label);

  gtk_widget_class_bind_template_child(widget_class, GateBox, attack_zone_start_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, attack_threshold_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, release_zone_start_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, release_threshold_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, gain_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, sidechain_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, curve_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, envelope_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, attack);
  gtk_widget_class_bind_template_child(widget_class, GateBox, release);
  gtk_widget_class_bind_template_child(widget_class, GateBox, curve_threshold);
  gtk_widget_class_bind_template_child(widget_class, GateBox, curve_zone);
  gtk_widget_class_bind_template_child(widget_class, GateBox, hysteresis);
  gtk_widget_class_bind_template_child(widget_class, GateBox, hysteresis_threshold);
  gtk_widget_class_bind_template_child(widget_class, GateBox, hysteresis_zone);
  gtk_widget_class_bind_template_child(widget_class, GateBox, dry);
  gtk_widget_class_bind_template_child(widget_class, GateBox, wet);
  gtk_widget_class_bind_template_child(widget_class, GateBox, reduction);
  gtk_widget_class_bind_template_child(widget_class, GateBox, makeup);
  gtk_widget_class_bind_template_child(widget_class, GateBox, preamp);
  gtk_widget_class_bind_template_child(widget_class, GateBox, reactivity);
  gtk_widget_class_bind_template_child(widget_class, GateBox, lookahead);
  gtk_widget_class_bind_template_child(widget_class, GateBox, hpf_freq);
  gtk_widget_class_bind_template_child(widget_class, GateBox, lpf_freq);
  gtk_widget_class_bind_template_child(widget_class, GateBox, sidechain_input);
  gtk_widget_class_bind_template_child(widget_class, GateBox, sidechain_mode);
  gtk_widget_class_bind_template_child(widget_class, GateBox, sidechain_source);
  gtk_widget_class_bind_template_child(widget_class, GateBox, lpf_mode);
  gtk_widget_class_bind_template_child(widget_class, GateBox, hpf_mode);
  gtk_widget_class_bind_template_child(widget_class, GateBox, listen);
  gtk_widget_class_bind_template_child(widget_class, GateBox, dropdown_input_devices);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
  gtk_widget_class_bind_template_callback(widget_class, set_dropdown_sensitive);
}

void gate_box_init(GateBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  prepare_spinbuttons<"dB">(self->curve_threshold, self->curve_zone, self->hysteresis_threshold, self->hysteresis_zone,
                            self->reduction, self->makeup, self->preamp);

  prepare_spinbuttons<"Hz">(self->hpf_freq, self->lpf_freq);

  prepare_spinbuttons<"ms">(self->attack, self->release, self->lookahead, self->reactivity);

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  // The following spinbuttons can assume -inf
  prepare_spinbuttons<"dB", false>(self->dry, self->wet);
}

auto create() -> GateBox* {
  return static_cast<GateBox*>(g_object_new(EE_TYPE_GATE_BOX, nullptr));
}

}  // namespace ui::gate_box
