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

#include "deesser_ui.hpp"

namespace ui::deesser_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<Deesser> deesser;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _DeesserBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkLevelBar *compression, *detected;

  GtkLabel *compression_label, *detected_label;

  GtkSpinButton *f1_freq, *f2_freq, *f1_level, *f2_level, *f2_q, *threshold, *ratio, *laxity, *makeup;

  GtkToggleButton* sc_listen;

  GtkComboBoxText *detection, *mode;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(DeesserBox, deesser_box, GTK_TYPE_BOX)

void on_reset(DeesserBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void setup(DeesserBox* self, std::shared_ptr<Deesser> deesser, const std::string& schema_path) {
  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->data->deesser = deesser;

  self->settings = g_settings_new_with_path(tags::schema::deesser::id, schema_path.c_str());

  deesser->set_post_messages(true);

  self->data->connections.push_back(deesser->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(deesser->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(deesser->detected.connect([=](const double value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LEVEL_BAR(self->compression) || !GTK_IS_LABEL(self->compression_label)) {
        return;
      }

      gtk_level_bar_set_value(self->compression, 1.0 - value);
      gtk_label_set_text(self->compression_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  self->data->connections.push_back(deesser->compression.connect([=](const double value) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      if (!GTK_IS_LEVEL_BAR(self->detected) || !GTK_IS_LABEL(self->detected_label)) {
        return;
      }

      gtk_level_bar_set_value(self->detected, value);
      gtk_label_set_text(self->detected_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
    });
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "makeup", gtk_spin_button_get_adjustment(self->makeup), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "ratio", gtk_spin_button_get_adjustment(self->ratio), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "threshold", gtk_spin_button_get_adjustment(self->threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "f1-freq", gtk_spin_button_get_adjustment(self->f1_freq), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "f2-freq", gtk_spin_button_get_adjustment(self->f2_freq), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "f1-level", gtk_spin_button_get_adjustment(self->f1_level), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "f2-level", gtk_spin_button_get_adjustment(self->f2_level), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "f2-q", gtk_spin_button_get_adjustment(self->f2_q), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "laxity", gtk_spin_button_get_adjustment(self->laxity), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "sc-listen", self->sc_listen, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "detection", self->detection, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "mode", self->mode, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_DEESSER_BOX(object);

  self->data->deesser->set_post_messages(false);

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

  G_OBJECT_CLASS(deesser_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_DEESSER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(deesser_box_parent_class)->finalize(object);
}

void deesser_box_class_init(DeesserBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->finalize = finalize;
  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::deesser_ui);

  gtk_widget_class_bind_template_child(widget_class, DeesserBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, DeesserBox, compression);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, compression_label);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, detected);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, detected_label);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, sc_listen);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, detection);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, mode);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, f1_freq);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, f2_freq);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, f1_level);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, f2_level);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, f2_q);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, ratio);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, laxity);
  gtk_widget_class_bind_template_child(widget_class, DeesserBox, makeup);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void deesser_box_init(DeesserBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbuttons<"dB">(self->makeup, self->threshold, self->f1_level, self->f2_level);

  prepare_spinbuttons<"Hz">(self->f1_freq, self->f2_freq);

  prepare_spinbuttons<"">(self->f2_q);

  prepare_scales<"dB">(self->input_gain, self->output_gain);
}

auto create() -> DeesserBox* {
  return static_cast<DeesserBox*>(g_object_new(EE_TYPE_DEESSER_BOX, nullptr));
}

}  // namespace ui::deesser_box
