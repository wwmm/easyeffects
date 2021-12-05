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

#include "bass_loudness_ui.hpp"

namespace ui::bass_loudness_box {

using namespace std::string_literals;

auto constexpr log_tag = "bass_loudness_box: ";

struct _BassLoudnessBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkSpinButton *loudness, *output, *link;

  GSettings* settings;

  std::shared_ptr<BassLoudness> bass_loudness;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(BassLoudnessBox, bass_loudness_box, GTK_TYPE_BOX)

void on_bypass(BassLoudnessBox* self, GtkToggleButton* btn) {
  self->bass_loudness->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(BassLoudnessBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "loudness");

  g_settings_reset(self->settings, "output");

  g_settings_reset(self->settings, "link");
}

void setup(BassLoudnessBox* self, std::shared_ptr<BassLoudness> bass_loudness, const std::string& schema_path) {
  self->bass_loudness = bass_loudness;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.bassloudness", schema_path.c_str());

  bass_loudness->post_messages = true;
  bass_loudness->bypass = false;

  self->connections.push_back(bass_loudness->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(bass_loudness->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "loudness", gtk_spin_button_get_adjustment(self->loudness), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "output", gtk_spin_button_get_adjustment(self->output), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "link", gtk_spin_button_get_adjustment(self->link), "value", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_BASS_LOUDNESS_BOX(object);

  self->bass_loudness->post_messages = false;
  self->bass_loudness->bypass = false;

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->connections.clear();
  self->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(bass_loudness_box_parent_class)->dispose(object);
}

void bass_loudness_box_class_init(BassLoudnessBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/bass_loudness.ui");

  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, loudness);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, output);
  gtk_widget_class_bind_template_child(widget_class, BassLoudnessBox, link);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void bass_loudness_box_init(BassLoudnessBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  prepare_spinbutton<"dB">(self->loudness);
  prepare_spinbutton<"dB">(self->output);
  prepare_spinbutton<"dB">(self->link);
}

auto create() -> BassLoudnessBox* {
  return static_cast<BassLoudnessBox*>(g_object_new(EE_TYPE_BASS_LOUDNESS_BOX, nullptr));
}

}  // namespace ui::bass_loudness_box
