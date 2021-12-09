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

#include "echo_canceller_ui.hpp"

namespace ui::echo_canceller_box {

using namespace std::string_literals;

auto constexpr log_tag = "echo_canceller_box: ";

struct _EchoCancellerBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkSpinButton *frame_size, *filter_length;

  GSettings* settings;

  std::shared_ptr<EchoCanceller> echo_canceller;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(EchoCancellerBox, echo_canceller_box, GTK_TYPE_BOX)

void on_bypass(EchoCancellerBox* self, GtkToggleButton* btn) {
  self->echo_canceller->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(EchoCancellerBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "frame-size");

  g_settings_reset(self->settings, "filter-length");
}

void setup(EchoCancellerBox* self, std::shared_ptr<EchoCanceller> echo_canceller, const std::string& schema_path) {
  self->echo_canceller = echo_canceller;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.echocanceller", schema_path.c_str());

  echo_canceller->post_messages = true;
  echo_canceller->bypass = false;

  self->connections.push_back(echo_canceller->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(echo_canceller->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "frame-size", gtk_spin_button_get_adjustment(self->frame_size), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "filter-length", gtk_spin_button_get_adjustment(self->filter_length), "value",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_ECHO_CANCELLER_BOX(object);

  self->echo_canceller->bypass = false;

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

  G_OBJECT_CLASS(echo_canceller_box_parent_class)->dispose(object);
}

void echo_canceller_box_class_init(EchoCancellerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/echo_canceller.ui");

  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, frame_size);
  gtk_widget_class_bind_template_child(widget_class, EchoCancellerBox, filter_length);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void echo_canceller_box_init(EchoCancellerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  prepare_spinbutton<"ms">(self->filter_length);
  prepare_spinbutton<"ms">(self->frame_size);
}

auto create() -> EchoCancellerBox* {
  return static_cast<EchoCancellerBox*>(g_object_new(EE_TYPE_ECHO_CANCELLER_BOX, nullptr));
}

}  // namespace ui::echo_canceller_box
