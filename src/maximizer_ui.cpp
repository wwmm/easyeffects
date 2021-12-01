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

#include "maximizer_ui.hpp"

namespace ui::maximizer_box {

using namespace std::string_literals;

auto constexpr log_tag = "maximizer_box: ";

struct _MaximizerBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkSpinButton *release, *threshold, *ceiling;

  GtkLevelBar* reduction_levelbar;

  GtkLabel* reduction_label;

  GSettings* settings;

  std::shared_ptr<Maximizer> maximizer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(MaximizerBox, maximizer_box, GTK_TYPE_BOX)

void on_bypass(MaximizerBox* self, GtkToggleButton* btn) {
  self->maximizer->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(MaximizerBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "release");

  g_settings_reset(self->settings, "ceiling");

  g_settings_reset(self->settings, "threshold");
}

void setup(MaximizerBox* self, std::shared_ptr<Maximizer> maximizer, const std::string& schema_path) {
  self->maximizer = maximizer;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.maximizer", schema_path.c_str());

  maximizer->post_messages = true;
  maximizer->bypass = false;

  self->connections.push_back(maximizer->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(maximizer->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  self->connections.push_back(maximizer->reduction.connect([=](const double& value) {
    gtk_level_bar_set_value(self->reduction_levelbar, value);
    gtk_label_set_text(self->reduction_label, fmt::format("{0:.0f}", value).c_str());
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "ceiling", gtk_spin_button_get_adjustment(self->ceiling), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "release", gtk_spin_button_get_adjustment(self->release), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "threshold", gtk_spin_button_get_adjustment(self->threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MAXIMIZER_BOX(object);

  self->maximizer->post_messages = false;
  self->maximizer->bypass = false;

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

  G_OBJECT_CLASS(maximizer_box_parent_class)->dispose(object);
}

void maximizer_box_class_init(MaximizerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/maximizer.ui");

  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, release);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, ceiling);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, reduction_levelbar);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, reduction_label);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void maximizer_box_init(MaximizerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  prepare_spinbutton<"dB">(self->threshold);
  prepare_spinbutton<"dB">(self->ceiling);
  prepare_spinbutton<"ms">(self->release);
}

auto create() -> MaximizerBox* {
  return static_cast<MaximizerBox*>(g_object_new(EE_TYPE_MAXIMIZER_BOX, nullptr));
}

}  // namespace ui::maximizer_box
