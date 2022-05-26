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

#include "loudness_ui.hpp"

namespace ui::loudness_box {

using namespace std::string_literals;

auto constexpr log_tag = "loudness_box: ";

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  std::shared_ptr<Loudness> loudness;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _LoudnessBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkComboBoxText *fft_size, *standard;

  GtkSpinButton* volume;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(LoudnessBox, loudness_box, GTK_TYPE_BOX)

void on_bypass(LoudnessBox* self, GtkToggleButton* btn) {
  self->data->loudness->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(LoudnessBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
}

void setup(LoudnessBox* self, std::shared_ptr<Loudness> loudness, const std::string& schema_path) {
  self->data->loudness = loudness;

  self->settings = g_settings_new_with_path((tags::app::id + ".loudness").c_str(), schema_path.c_str());

  loudness->post_messages = true;
  loudness->bypass = false;

  self->data->connections.push_back(loudness->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(loudness->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "volume", gtk_spin_button_get_adjustment(self->volume), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "fft", self->fft_size, "active-id", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "std", self->standard, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_LOUDNESS_BOX(object);

  self->data->loudness->bypass = false;

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(loudness_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_LOUDNESS_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(loudness_box_parent_class)->finalize(object);
}

void loudness_box_class_init(LoudnessBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, (tags::app::path + "/ui/loudness.ui").c_str());

  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, volume);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, standard);
  gtk_widget_class_bind_template_child(widget_class, LoudnessBox, fft_size);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void loudness_box_init(LoudnessBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"dB">(self->volume);
}

auto create() -> LoudnessBox* {
  return static_cast<LoudnessBox*>(g_object_new(EE_TYPE_LOUDNESS_BOX, nullptr));
}

}  // namespace ui::loudness_box
