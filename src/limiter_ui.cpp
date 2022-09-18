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

#include "limiter_ui.hpp"

namespace ui::limiter_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<Limiter> limiter;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _LimiterBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkComboBoxText *mode, *oversampling, *dither;

  GtkDropDown* dropdown_input_devices;

  GtkLabel *gain_left, *gain_right, *sidechain_left, *sidechain_right;

  GtkSpinButton *sc_preamp, *lookahead, *attack, *release, *threshold, *stereo_link, *alr_attack, *alr_release,
      *alr_knee;

  GtkCheckButton* gain_boost;

  GtkToggleButton *alr, *external_sidechain;

  GListStore* input_devices_model;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(LimiterBox, limiter_box, GTK_TYPE_BOX)

void on_reset(LimiterBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void setup_dropdown_input_device(LimiterBox* self) {
  auto* selection = gtk_single_selection_new(G_LIST_MODEL(self->input_devices_model));

  g_signal_connect(self->dropdown_input_devices, "notify::selected-item",
                   G_CALLBACK(+[](GtkDropDown* dropdown, GParamSpec* pspec, LimiterBox* self) {
                     if (auto selected_item = gtk_drop_down_get_selected_item(dropdown); selected_item != nullptr) {
                       auto* holder = static_cast<ui::holders::NodeInfoHolder*>(selected_item);

                       g_settings_set_string(self->settings, "sidechain-input-device", holder->info->name.c_str());
                     }
                   }),
                   self);

  gtk_drop_down_set_model(self->dropdown_input_devices, G_LIST_MODEL(self->input_devices_model));

  g_object_unref(selection);
}

void setup(LimiterBox* self, std::shared_ptr<Limiter> limiter, const std::string& schema_path, PipeManager* pm) {
  self->data->limiter = limiter;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::limiter::id, schema_path.c_str());

  limiter->set_post_messages(true);

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

  self->data->connections.push_back(limiter->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(limiter->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(limiter->gain_left.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->gain_left)) {
        return;
      }

      gtk_label_set_text(self->gain_left, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(limiter->gain_right.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->gain_right)) {
        return;
      }

      gtk_label_set_text(self->gain_right, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(limiter->sidechain_left.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->sidechain_left)) {
        return;
      }

      gtk_label_set_text(self->sidechain_left, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(limiter->sidechain_right.connect([=](const float value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LABEL(self->sidechain_right)) {
        return;
      }

      gtk_label_set_text(self->sidechain_right, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
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

    auto holder = ui::holders::create(info);

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

  g_settings_bind(self->settings, "sidechain-preamp", gtk_spin_button_get_adjustment(self->sc_preamp), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "lookahead", gtk_spin_button_get_adjustment(self->lookahead), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "attack", gtk_spin_button_get_adjustment(self->attack), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "release", gtk_spin_button_get_adjustment(self->release), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "threshold", gtk_spin_button_get_adjustment(self->threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "stereo-link", gtk_spin_button_get_adjustment(self->stereo_link), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "alr-attack", gtk_spin_button_get_adjustment(self->alr_attack), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "alr-release", gtk_spin_button_get_adjustment(self->alr_release), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "alr-knee", gtk_spin_button_get_adjustment(self->alr_knee), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "gain-boost", self->gain_boost, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "external-sidechain", self->external_sidechain, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "alr", self->alr, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "mode", self->mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "oversampling", self->oversampling, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "dithering", self->dither, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_LIMITER_BOX(object);

  self->data->limiter->set_post_messages(false);

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

  G_OBJECT_CLASS(limiter_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_LIMITER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(limiter_box_parent_class)->finalize(object);
}

void limiter_box_class_init(LimiterBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::limiter_ui);

  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, LimiterBox, mode);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, oversampling);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, dither);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, sc_preamp);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, lookahead);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, attack);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, release);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, stereo_link);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr_attack);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr_release);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr_knee);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, alr);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, external_sidechain);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, gain_boost);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, gain_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, gain_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, sidechain_left);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, sidechain_right);
  gtk_widget_class_bind_template_child(widget_class, LimiterBox, dropdown_input_devices);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void limiter_box_init(LimiterBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->input_devices_model = g_list_store_new(ui::holders::node_info_holder_get_type());

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"dB">(self->sc_preamp, self->threshold, self->alr_knee);

  prepare_spinbuttons<"ms">(self->lookahead, self->attack, self->release, self->alr_attack, self->alr_release);

  prepare_spinbuttons<"%">(self->stereo_link);
}

auto create() -> LimiterBox* {
  return static_cast<LimiterBox*>(g_object_new(EE_TYPE_LIMITER_BOX, nullptr));
}

}  // namespace ui::limiter_box
