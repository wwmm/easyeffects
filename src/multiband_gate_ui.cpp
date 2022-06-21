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

#include "multiband_gate_ui.hpp"

namespace ui::multiband_gate_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  std::shared_ptr<MultibandGate> multiband_gate;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _MultibandGateBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkLevelBar *output0, *output1, *output2, *output3, *gating0, *gating1, *gating2, *gating3;

  GtkLabel *output0_label, *output1_label, *output2_label, *output3_label, *gating0_label, *gating1_label,
      *gating2_label, *gating3_label;

  GtkSpinButton *freq0, *freq1, *freq2, *range0, *range1, *range2, *range3, *attack0, *attack1, *attack2, *attack3,
      *release0, *release1, *release2, *release3, *threshold0, *threshold1, *threshold2, *threshold3, *knee0, *knee1,
      *knee2, *knee3, *ratio0, *ratio1, *ratio2, *ratio3, *makeup0, *makeup1, *makeup2, *makeup3;

  GtkComboBoxText *mode, *detection0, *detection1, *detection2, *detection3;

  GtkToggleButton *bypass0, *bypass1, *bypass2, *bypass3, *solo0, *solo1, *solo2, *solo3;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(MultibandGateBox, multiband_gate_box, GTK_TYPE_BOX)

void on_bypass(MultibandGateBox* self, GtkToggleButton* btn) {
  self->data->multiband_gate->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(MultibandGateBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
}

void setup(MultibandGateBox* self, std::shared_ptr<MultibandGate> multiband_gate, const std::string& schema_path) {
  self->data->multiband_gate = multiband_gate;

  self->settings = g_settings_new_with_path(tags::schema::multiband_gate::id, schema_path.c_str());

  multiband_gate->post_messages = true;
  multiband_gate->bypass = false;

  self->data->connections.push_back(multiband_gate->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(multiband_gate->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  self->data->connections.push_back(multiband_gate->output0.connect([=](const double& value) {
    gtk_level_bar_set_value(self->output0, value);
    gtk_label_set_text(self->output0_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->output1.connect([=](const double& value) {
    gtk_level_bar_set_value(self->output1, value);
    gtk_label_set_text(self->output1_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->output2.connect([=](const double& value) {
    gtk_level_bar_set_value(self->output2, value);
    gtk_label_set_text(self->output2_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->output3.connect([=](const double& value) {
    gtk_level_bar_set_value(self->output3, value);
    gtk_label_set_text(self->output3_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->gating0.connect([=](const double& value) {
    gtk_level_bar_set_value(self->gating0, 1.0 - value);
    gtk_label_set_text(self->gating0_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->gating1.connect([=](const double& value) {
    gtk_level_bar_set_value(self->gating1, 1.0 - value);
    gtk_label_set_text(self->gating1_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->gating2.connect([=](const double& value) {
    gtk_level_bar_set_value(self->gating2, 1.0 - value);
    gtk_label_set_text(self->gating2_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  self->data->connections.push_back(multiband_gate->gating3.connect([=](const double& value) {
    gtk_level_bar_set_value(self->gating3, 1.0 - value);
    gtk_label_set_text(self->gating3_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  gsettings_bind_widgets<"freq0", "freq1", "freq2">(self->settings, self->freq0, self->freq1, self->freq2);

  gsettings_bind_widgets<"threshold0", "threshold1", "threshold2", "threshold3">(
      self->settings, self->threshold0, self->threshold1, self->threshold2, self->threshold3);

  gsettings_bind_widgets<"ratio0", "ratio1", "ratio2", "ratio3">(self->settings, self->ratio0, self->ratio1,
                                                                 self->ratio2, self->ratio3);

  gsettings_bind_widgets<"range0", "range1", "range2", "range3">(self->settings, self->range0, self->range1,
                                                                 self->range2, self->range3);

  gsettings_bind_widgets<"bypass0", "bypass1", "bypass2", "bypass3">(self->settings, self->bypass0, self->bypass1,
                                                                     self->bypass2, self->bypass3);

  gsettings_bind_widgets<"solo0", "solo1", "solo2", "solo3">(self->settings, self->solo0, self->solo1, self->solo2,
                                                             self->solo3);

  gsettings_bind_widgets<"attack0", "attack1", "attack2", "attack3">(self->settings, self->attack0, self->attack1,
                                                                     self->attack2, self->attack3);

  gsettings_bind_widgets<"release0", "release1", "release2", "release3">(self->settings, self->release0, self->release1,
                                                                         self->release2, self->release3);

  gsettings_bind_widgets<"makeup0", "makeup1", "makeup2", "makeup3">(self->settings, self->makeup0, self->makeup1,
                                                                     self->makeup2, self->makeup3);

  gsettings_bind_widgets<"knee0", "knee1", "knee2", "knee3">(self->settings, self->knee0, self->knee1, self->knee2,
                                                             self->knee3);

  gsettings_bind_widget(self->settings, "mode", self->mode);

  gsettings_bind_widgets<"detection0", "detection1", "detection2", "detection3">(
      self->settings, self->detection0, self->detection1, self->detection2, self->detection3);
}

void dispose(GObject* object) {
  auto* self = EE_MULTIBAND_GATE_BOX(object);

  self->data->multiband_gate->bypass = false;

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

  G_OBJECT_CLASS(multiband_gate_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MULTIBAND_GATE_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(multiband_gate_box_parent_class)->finalize(object);
}

void multiband_gate_box_class_init(MultibandGateBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::multiband_gate_ui);

  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output0_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output1_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output2_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, output3_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating0_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating1_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating2_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, gating3_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, freq0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, freq1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, freq2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, range0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, range1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, range2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, range3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, attack0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, attack1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, attack2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, attack3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, release0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, release1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, release2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, release3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, threshold0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, threshold1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, threshold2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, threshold3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, knee0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, knee1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, knee2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, knee3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, ratio0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, ratio1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, ratio2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, ratio3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, makeup0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, makeup1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, makeup2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, makeup3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, bypass0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, bypass1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, bypass2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, bypass3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, solo0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, solo1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, solo2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, solo3);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, mode);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, detection0);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, detection1);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, detection2);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBox, detection3);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void multiband_gate_box_init(MultibandGateBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"dB">(self->range0, self->range1, self->range2, self->range3);
  prepare_spinbuttons<"dB">(self->threshold0, self->threshold1, self->threshold2, self->threshold3);
  prepare_spinbuttons<"dB">(self->knee0, self->knee1, self->knee2, self->knee3);
  prepare_spinbuttons<"dB">(self->makeup0, self->makeup1, self->makeup2, self->makeup3);
  prepare_spinbuttons<"Hz">(self->freq0, self->freq1, self->freq2);
  prepare_spinbuttons<"ms">(self->attack0, self->attack1, self->attack2, self->attack3);
  prepare_spinbuttons<"ms">(self->release0, self->release1, self->release2, self->release3);
  prepare_spinbuttons<"">(self->ratio0, self->ratio1, self->ratio2, self->ratio3);
}

auto create() -> MultibandGateBox* {
  return static_cast<MultibandGateBox*>(g_object_new(EE_TYPE_MULTIBAND_GATE_BOX, nullptr));
}

}  // namespace ui::multiband_gate_box
