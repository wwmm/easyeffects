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

#include "speex_ui.hpp"

namespace ui::speex_box {

using namespace std::string_literals;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0;

  app::Application* application;

  std::shared_ptr<Speex> speex;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _SpeexBox {
  GtkBox parent_instance;

  AdwToastOverlay* toast_overlay;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label,
      *noise_suppression_label;

  GtkSwitch *enable_denoise, *enable_agc, *enable_vad, *enable_dereverb;

  GtkSpinButton *noise_suppression, *vad_probability_start, *vad_probability_continue;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(SpeexBox, speex_box, GTK_TYPE_BOX)

void on_reset(SpeexBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void setup(SpeexBox* self,
           std::shared_ptr<Speex> speex,
           const std::string& schema_path,
           app::Application* application) {
  self->data->speex = speex;
  self->data->application = application;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::speex::id, schema_path.c_str());

  speex->set_post_messages(true);

  self->data->connections.push_back(speex->input_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                   self->input_level_right_label, left, right);
    });
  }));

  self->data->connections.push_back(speex->output_level.connect([=](const float left, const float right) {
    util::idle_add([=]() {
      if (get_ignore_filter_idle_add(serial)) {
        return;
      }

      update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                   self->output_level_right_label, left, right);
    });
  }));

  gsettings_bind_widgets<"input-gain", "output-gain", "enable-denoise", "noise-suppression", "enable-agc", "enable-vad",
                         "vad-probability-start", "vad-probability-continue", "enable-dereverb">(
      self->settings, self->input_gain, self->output_gain, self->enable_denoise, self->noise_suppression,
      self->enable_agc, self->enable_vad, self->vad_probability_start, self->vad_probability_continue,
      self->enable_dereverb);
}

void dispose(GObject* object) {
  auto* self = EE_SPEEX_BOX(object);

  self->data->speex->set_post_messages(false);

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

  G_OBJECT_CLASS(speex_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_SPEEX_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(speex_box_parent_class)->finalize(object);
}

void speex_box_class_init(SpeexBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::speex_ui);

  gtk_widget_class_bind_template_child(widget_class, SpeexBox, toast_overlay);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, SpeexBox, enable_denoise);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, enable_agc);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, enable_vad);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, enable_dereverb);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, noise_suppression);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, vad_probability_start);
  gtk_widget_class_bind_template_child(widget_class, SpeexBox, vad_probability_continue);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void speex_box_init(SpeexBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"dB">(self->noise_suppression);

  prepare_spinbuttons<"%">(self->vad_probability_start);
  prepare_spinbuttons<"%">(self->vad_probability_continue);
}

auto create() -> SpeexBox* {
  return static_cast<SpeexBox*>(g_object_new(EE_TYPE_SPEEX_BOX, nullptr));
}

}  // namespace ui::speex_box
