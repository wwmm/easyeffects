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

#include "reverb_ui.hpp"

namespace ui::reverb_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  std::shared_ptr<Reverb> reverb;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _ReverbBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkComboBoxText* room_size;

  GtkSpinButton *predelay, *decay_time, *diffusion, *dry, *wet, *hf_damp, *bass_cut, *treble_cut;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(ReverbBox, reverb_box, GTK_TYPE_BOX)

void on_reset(ReverbBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void on_preset_room(ReverbBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->decay_time, 0.445945);
  gtk_spin_button_set_value(self->hf_damp, 5508.46);
  gtk_combo_box_set_active(GTK_COMBO_BOX(self->room_size), 4);
  gtk_spin_button_set_value(self->diffusion, 0.54);
  gtk_spin_button_set_value(self->wet, util::linear_to_db(0.469761));
  gtk_spin_button_set_value(self->dry, util::linear_to_db(1.0));
  gtk_spin_button_set_value(self->predelay, 25.0);
  gtk_spin_button_set_value(self->bass_cut, 257.65);
  gtk_spin_button_set_value(self->treble_cut, 20000.0);
}

void on_preset_empty_walls(ReverbBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->decay_time, 0.505687);
  gtk_spin_button_set_value(self->hf_damp, 3971.64);
  gtk_combo_box_set_active(GTK_COMBO_BOX(self->room_size), 4);
  gtk_spin_button_set_value(self->diffusion, 0.17);
  gtk_spin_button_set_value(self->wet, util::linear_to_db(0.198884));
  gtk_spin_button_set_value(self->dry, util::linear_to_db(1.0));
  gtk_spin_button_set_value(self->predelay, 13.0);
  gtk_spin_button_set_value(self->bass_cut, 240.453);
  gtk_spin_button_set_value(self->treble_cut, 3303.47);
}

void on_preset_ambience(ReverbBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->decay_time, 1.10354);
  gtk_spin_button_set_value(self->hf_damp, 2182.58);
  gtk_combo_box_set_active(GTK_COMBO_BOX(self->room_size), 4);
  gtk_spin_button_set_value(self->diffusion, 0.69);
  gtk_spin_button_set_value(self->wet, util::linear_to_db(0.291183));
  gtk_spin_button_set_value(self->dry, util::linear_to_db(1.0));
  gtk_spin_button_set_value(self->predelay, 6.5);
  gtk_spin_button_set_value(self->bass_cut, 514.079);
  gtk_spin_button_set_value(self->treble_cut, 4064.15);
}

void on_preset_large_empty_hall(ReverbBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->decay_time, 2.00689);
  gtk_spin_button_set_value(self->hf_damp, 20000.0);
  gtk_spin_button_set_value(self->wet, util::linear_to_db(0.366022));
  g_settings_reset(self->settings, "room-size");
  g_settings_reset(self->settings, "diffusion");
  g_settings_reset(self->settings, "dry");
  g_settings_reset(self->settings, "predelay");
  g_settings_reset(self->settings, "bass-cut");
  g_settings_reset(self->settings, "treble-cut");
}

void on_preset_disco(ReverbBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->decay_time, 1.0);
  gtk_spin_button_set_value(self->hf_damp, 3396.49);
  gtk_spin_button_set_value(self->wet, util::linear_to_db(0.269807));
  g_settings_reset(self->settings, "room-size");
  g_settings_reset(self->settings, "diffusion");
  g_settings_reset(self->settings, "dry");
  g_settings_reset(self->settings, "predelay");
  g_settings_reset(self->settings, "bass-cut");
  g_settings_reset(self->settings, "treble-cut");
}

void on_preset_large_occupied_hall(ReverbBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->decay_time, 1.45397);
  gtk_spin_button_set_value(self->hf_damp, 9795.58);
  gtk_spin_button_set_value(self->wet, util::linear_to_db(0.184284));
  g_settings_reset(self->settings, "room-size");
  g_settings_reset(self->settings, "diffusion");
  g_settings_reset(self->settings, "dry");
  g_settings_reset(self->settings, "predelay");
  g_settings_reset(self->settings, "bass-cut");
  g_settings_reset(self->settings, "treble-cut");
}

void setup(ReverbBox* self, std::shared_ptr<Reverb> reverb, const std::string& schema_path) {
  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->data->reverb = reverb;

  self->settings = g_settings_new_with_path(tags::schema::reverb::id, schema_path.c_str());

  reverb->set_post_messages(true);

  self->data->connections.push_back(reverb->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(reverb->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "amount", gtk_spin_button_get_adjustment(self->wet), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "predelay", gtk_spin_button_get_adjustment(self->predelay), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "decay-time", gtk_spin_button_get_adjustment(self->decay_time), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "diffusion", gtk_spin_button_get_adjustment(self->diffusion), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "dry", gtk_spin_button_get_adjustment(self->dry), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "hf-damp", gtk_spin_button_get_adjustment(self->hf_damp), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "bass-cut", gtk_spin_button_get_adjustment(self->bass_cut), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "treble-cut", gtk_spin_button_get_adjustment(self->treble_cut), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "room-size", self->room_size, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_REVERB_BOX(object);

  self->data->reverb->set_post_messages(false);

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

  G_OBJECT_CLASS(reverb_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_REVERB_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(reverb_box_parent_class)->finalize(object);
}

void reverb_box_class_init(ReverbBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::reverb_ui);

  gtk_widget_class_bind_template_child(widget_class, ReverbBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, ReverbBox, room_size);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, predelay);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, decay_time);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, diffusion);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, dry);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, wet);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, hf_damp);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, bass_cut);
  gtk_widget_class_bind_template_child(widget_class, ReverbBox, treble_cut);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_room);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_empty_walls);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_ambience);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_large_empty_hall);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_disco);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_large_occupied_hall);
}

void reverb_box_init(ReverbBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"Hz">(self->hf_damp, self->bass_cut, self->treble_cut);

  prepare_spinbuttons<"s">(self->decay_time);

  prepare_spinbuttons<"ms">(self->predelay);

  prepare_spinbuttons<"">(self->diffusion);

  // These spinbuttons can assume -inf
  prepare_spinbuttons<"dB", false>(self->wet, self->dry);
}

auto create() -> ReverbBox* {
  return static_cast<ReverbBox*>(g_object_new(EE_TYPE_REVERB_BOX, nullptr));
}

}  // namespace ui::reverb_box
