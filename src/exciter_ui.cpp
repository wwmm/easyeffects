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

#include "exciter_ui.hpp"

namespace ui::exciter_box {

using namespace std::string_literals;

auto constexpr log_tag = "exciter_box: ";

struct _ExciterBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkLevelBar* harmonics_levelbar;

  GtkLabel* harmonics_levelbar_label;

  GtkSpinButton *ceil, *amount, *harmonics, *scope;

  GtkScale* blend;

  GtkToggleButton *ceil_active, *listen;

  GSettings* settings;

  std::shared_ptr<Exciter> exciter;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(ExciterBox, exciter_box, GTK_TYPE_BOX)

void on_bypass(ExciterBox* self, GtkToggleButton* btn) {
  self->exciter->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(ExciterBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "amount");

  g_settings_reset(self->settings, "harmonics");

  g_settings_reset(self->settings, "scope");

  g_settings_reset(self->settings, "ceil");

  g_settings_reset(self->settings, "blend");

  g_settings_reset(self->settings, "ceil-active");

  g_settings_reset(self->settings, "listen");
}

void setup(ExciterBox* self, std::shared_ptr<Exciter> exciter, const std::string& schema_path) {
  self->exciter = exciter;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.exciter", schema_path.c_str());

  exciter->post_messages = true;
  exciter->bypass = false;

  self->connections.push_back(exciter->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(exciter->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  self->connections.push_back(exciter->harmonics.connect([=](const double& value) {
    gtk_level_bar_set_value(self->harmonics_levelbar, value);
    gtk_label_set_text(self->harmonics_levelbar_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "amount", gtk_spin_button_get_adjustment(self->amount), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "harmonics", gtk_spin_button_get_adjustment(self->harmonics), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "scope", gtk_spin_button_get_adjustment(self->scope), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "ceil", gtk_spin_button_get_adjustment(self->ceil), "value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "blend", gtk_range_get_adjustment(GTK_RANGE(self->blend)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "listen", self->listen, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "ceil-active", self->ceil_active, "active", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_EXCITER_BOX(object);

  self->exciter->post_messages = false;
  self->exciter->bypass = false;

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

  G_OBJECT_CLASS(exciter_box_parent_class)->dispose(object);
}

void exciter_box_class_init(ExciterBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/exciter.ui");

  gtk_widget_class_bind_template_child(widget_class, ExciterBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, ExciterBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, ExciterBox, harmonics_levelbar_label);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, harmonics_levelbar);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, ceil);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, amount);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, harmonics);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, scope);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, blend);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, ceil_active);
  gtk_widget_class_bind_template_child(widget_class, ExciterBox, listen);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void exciter_box_init(ExciterBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  prepare_spinbutton<"dB">(self->amount);
  prepare_spinbutton<"Hz">(self->scope);
  prepare_spinbutton<"Hz">(self->ceil);
  prepare_spinbutton<"">(self->harmonics);

  prepare_scale<"">(self->blend);
}

auto create() -> ExciterBox* {
  return static_cast<ExciterBox*>(g_object_new(EE_TYPE_EXCITER_BOX, nullptr));
}

}  // namespace ui::exciter_box
