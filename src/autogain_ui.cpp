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

#include "autogain_ui.hpp"

namespace ui::autogain_box {

using namespace std::string_literals;

auto constexpr log_tag = "autogain_box: ";

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  std::shared_ptr<AutoGain> autogain;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _AutogainBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkSpinButton *target, *maximum_history;

  GtkLevelBar *m_level, *s_level, *i_level, *r_level, *g_level, *l_level, *lra_level;

  GtkLabel *m_label, *s_label, *i_label, *r_label, *g_label, *l_label, *lra_label;

  GtkButton* reset_history;

  GtkComboBoxText* reference;

  GtkToggleButton* bypass;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(AutogainBox, autogain_box, GTK_TYPE_BOX)

void on_bypass(AutogainBox* self, GtkToggleButton* btn) {
  self->data->autogain->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(AutogainBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
}

void on_reset_history(AutogainBox* self, GtkButton* btn) {
  // it is ugly but will ensure that third party tools are able to reset this plugin history

  g_settings_set_boolean(self->settings, "reset-history", !g_settings_get_boolean(self->settings, "reset-history"));
}

void setup(AutogainBox* self, std::shared_ptr<AutoGain> autogain, const std::string& schema_path) {
  self->data->autogain = autogain;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.autogain", schema_path.c_str());

  autogain->post_messages = true;
  autogain->bypass = false;

  self->data->connections.push_back(autogain->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(autogain->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  self->data->connections.push_back(autogain->results.connect(
      [=](const double& loudness, const double& gain, const double& momentary, const double& shortterm,
          const double& integrated, const double& relative, const double& range) {
        gtk_level_bar_set_value(self->l_level, util::db_to_linear(loudness));
        gtk_label_set_text(self->l_label, fmt::format("{0:.0f}", loudness).c_str());

        gtk_level_bar_set_value(self->g_level, util::db_to_linear(gain));
        gtk_label_set_text(self->g_label, fmt::format("{0:.0f}", gain).c_str());

        gtk_level_bar_set_value(self->m_level, util::db_to_linear(momentary));
        gtk_label_set_text(self->m_label, fmt::format("{0:.0f}", momentary).c_str());

        gtk_level_bar_set_value(self->s_level, util::db_to_linear(shortterm));
        gtk_label_set_text(self->s_label, fmt::format("{0:.0f}", shortterm).c_str());

        gtk_level_bar_set_value(self->i_level, util::db_to_linear(integrated));
        gtk_label_set_text(self->i_label, fmt::format("{0:.0f}", integrated).c_str());

        gtk_level_bar_set_value(self->r_level, util::db_to_linear(relative));
        gtk_label_set_text(self->r_label, fmt::format("{0:.0f}", relative).c_str());

        gtk_level_bar_set_value(self->lra_level, util::db_to_linear(range));
        gtk_label_set_text(self->lra_label, fmt::format("{0:.0f}", range).c_str());
      }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  gsettings_bind_widgets<"target", "maximum-history", "reference">(self->settings, self->target, self->maximum_history,
                                                                   self->reference);
}

void dispose(GObject* object) {
  auto* self = EE_AUTOGAIN_BOX(object);

  self->data->autogain->bypass = false;

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

  G_OBJECT_CLASS(autogain_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_AUTOGAIN_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalize"s);

  G_OBJECT_CLASS(autogain_box_parent_class)->finalize(object);
}

void autogain_box_class_init(AutogainBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/autogain.ui");

  gtk_widget_class_bind_template_child(widget_class, AutogainBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, AutogainBox, bypass);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, target);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, maximum_history);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, reference);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, reset_history);

  gtk_widget_class_bind_template_child(widget_class, AutogainBox, m_level);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, s_level);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, i_level);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, r_level);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, g_level);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, l_level);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, lra_level);

  gtk_widget_class_bind_template_child(widget_class, AutogainBox, m_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, s_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, i_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, r_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, g_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, l_label);
  gtk_widget_class_bind_template_child(widget_class, AutogainBox, lra_label);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);

  gtk_widget_class_bind_template_callback(widget_class, on_reset_history);
}

void autogain_box_init(AutogainBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"dB">(self->target);
  prepare_spinbutton<"s">(self->maximum_history);
}

auto create() -> AutogainBox* {
  return static_cast<AutogainBox*>(g_object_new(EE_TYPE_AUTOGAIN_BOX, nullptr));
}

}  // namespace ui::autogain_box
