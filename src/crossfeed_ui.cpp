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

#include "crossfeed_ui.hpp"

namespace ui::crossfeed_box {

using namespace std::string_literals;

auto constexpr log_tag = "crossfeed_box: ";

struct _CrossfeedBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkSpinButton *fcut, *feed;

  GSettings* settings;

  std::shared_ptr<Crossfeed> crossfeed;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(CrossfeedBox, crossfeed_box, GTK_TYPE_BOX)

void on_bypass(CrossfeedBox* self, GtkToggleButton* btn) {
  self->crossfeed->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(CrossfeedBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "fcut");

  g_settings_reset(self->settings, "feed");
}

void on_preset_cmoy(CrossfeedBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->fcut, 700.0);
  gtk_spin_button_set_value(self->feed, 6.0);
}

void on_preset_default(CrossfeedBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->fcut, 700.0);
  gtk_spin_button_set_value(self->feed, 4.5);
}

void on_preset_jmeier(CrossfeedBox* self, GtkButton* btn) {
  gtk_spin_button_set_value(self->fcut, 650.0);
  gtk_spin_button_set_value(self->feed, 9.0);
}

void setup(CrossfeedBox* self, std::shared_ptr<Crossfeed> crossfeed, const std::string& schema_path) {
  self->crossfeed = crossfeed;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.crossfeed", schema_path.c_str());

  crossfeed->post_messages = true;
  crossfeed->bypass = false;

  self->connections.push_back(crossfeed->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(crossfeed->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "fcut", gtk_spin_button_get_adjustment(self->fcut), "value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "feed", gtk_spin_button_get_adjustment(self->feed), "value", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_CROSSFEED_BOX(object);

  self->crossfeed->post_messages = false;
  self->crossfeed->bypass = false;

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

  G_OBJECT_CLASS(crossfeed_box_parent_class)->dispose(object);
}

void crossfeed_box_class_init(CrossfeedBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/crossfeed.ui");

  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, fcut);
  gtk_widget_class_bind_template_child(widget_class, CrossfeedBox, feed);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);

  gtk_widget_class_bind_template_callback(widget_class, on_preset_cmoy);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_default);
  gtk_widget_class_bind_template_callback(widget_class, on_preset_jmeier);
}

void crossfeed_box_init(CrossfeedBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  prepare_spinbutton<"Hz">(self->fcut);
  prepare_spinbutton<"dB">(self->feed);
}

auto create() -> CrossfeedBox* {
  return static_cast<CrossfeedBox*>(g_object_new(EE_TYPE_CROSSFEED_BOX, nullptr));
}

}  // namespace ui::crossfeed_box
