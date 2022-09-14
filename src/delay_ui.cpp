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

#include "delay_ui.hpp"

namespace ui::delay_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<Delay> delay;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _DelayBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkSpinButton *time_l, *time_r, *dry_l, *dry_r, *wet_l, *wet_r;

  GtkToggleButton *floor_active, *listen;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(DelayBox, delay_box, GTK_TYPE_BOX)

void on_reset(DelayBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void setup(DelayBox* self, std::shared_ptr<Delay> delay, const std::string& schema_path) {
  self->data->delay = delay;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::delay::id, schema_path.c_str());

  delay->set_post_messages(true);

  self->data->connections.push_back(delay->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(delay->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "time-l", gtk_spin_button_get_adjustment(self->time_l), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "time-r", gtk_spin_button_get_adjustment(self->time_r), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "dry-l", gtk_spin_button_get_adjustment(self->dry_l), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "dry-r", gtk_spin_button_get_adjustment(self->dry_r), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "wet-l", gtk_spin_button_get_adjustment(self->wet_l), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "wet-r", gtk_spin_button_get_adjustment(self->wet_r), "value",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_DELAY_BOX(object);

  self->data->delay->set_post_messages(false);

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

  G_OBJECT_CLASS(delay_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_DELAY_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(delay_box_parent_class)->finalize(object);
}

void delay_box_class_init(DelayBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::delay_ui);

  gtk_widget_class_bind_template_child(widget_class, DelayBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, DelayBox, time_l);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, time_r);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, dry_l);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, dry_r);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, wet_l);
  gtk_widget_class_bind_template_child(widget_class, DelayBox, wet_r);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void delay_box_init(DelayBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbuttons<"ms">(self->time_l, self->time_r);

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  // The following spinbuttons can assume -inf
  prepare_spinbuttons<"dB", false>(self->dry_l, self->dry_r, self->wet_l, self->wet_r);
}

auto create() -> DelayBox* {
  return static_cast<DelayBox*>(g_object_new(EE_TYPE_DELAY_BOX, nullptr));
}

}  // namespace ui::delay_box
