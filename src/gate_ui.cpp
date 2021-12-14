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

#include "gate_ui.hpp"

namespace ui::gate_box {

using namespace std::string_literals;

auto constexpr log_tag = "gate_box: ";

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  std::shared_ptr<Gate> gate;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _GateBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkLevelBar* gating;

  GtkLabel* gating_label;

  GtkSpinButton *attack, *release, *threshold, *knee, *ratio, *range, *makeup;

  GtkComboBoxText *detection, *stereo_link;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(GateBox, gate_box, GTK_TYPE_BOX)

void on_bypass(GateBox* self, GtkToggleButton* btn) {
  self->data->gate->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(GateBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
}

void setup(GateBox* self, std::shared_ptr<Gate> gate, const std::string& schema_path) {
  self->data->gate = gate;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.gate", schema_path.c_str());

  gate->post_messages = true;
  gate->bypass = false;

  self->data->connections.push_back(gate->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(gate->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  self->data->connections.push_back(gate->gating.connect([=](const double& value) {
    gtk_level_bar_set_value(self->gating, 1.0 - value);
    gtk_label_set_text(self->gating_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "attack", gtk_spin_button_get_adjustment(self->attack), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "range", gtk_spin_button_get_adjustment(self->range), "value",
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

  g_settings_bind(self->settings, "detection", self->detection, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "stereo-link", self->stereo_link, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_GATE_BOX(object);

  self->data->gate->bypass = false;

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

  G_OBJECT_CLASS(gate_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_GATE_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(gate_box_parent_class)->finalize(object);
}

void gate_box_class_init(GateBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/gate.ui");

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

  gtk_widget_class_bind_template_child(widget_class, GateBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, GateBox, gating);
  gtk_widget_class_bind_template_child(widget_class, GateBox, gating_label);
  gtk_widget_class_bind_template_child(widget_class, GateBox, attack);
  gtk_widget_class_bind_template_child(widget_class, GateBox, release);
  gtk_widget_class_bind_template_child(widget_class, GateBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, GateBox, knee);
  gtk_widget_class_bind_template_child(widget_class, GateBox, ratio);
  gtk_widget_class_bind_template_child(widget_class, GateBox, range);
  gtk_widget_class_bind_template_child(widget_class, GateBox, makeup);
  gtk_widget_class_bind_template_child(widget_class, GateBox, detection);
  gtk_widget_class_bind_template_child(widget_class, GateBox, stereo_link);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void gate_box_init(GateBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbutton<"dB">(self->range);
  prepare_spinbutton<"dB">(self->threshold);
  prepare_spinbutton<"dB">(self->knee);
  prepare_spinbutton<"dB">(self->makeup);
  prepare_spinbutton<"ms">(self->attack);
  prepare_spinbutton<"ms">(self->release);
  prepare_spinbutton<"">(self->ratio);
}

auto create() -> GateBox* {
  return static_cast<GateBox*>(g_object_new(EE_TYPE_GATE_BOX, nullptr));
}

}  // namespace ui::gate_box
