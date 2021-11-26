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

struct _AutogainBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkSpinButton* target;

  GtkLevelBar *m_level, *s_level, *i_level, *r_level, *g_level, *l_level, *lra_level;

  GtkLabel *m_label, *s_label, *i_label, *r_label, *g_label, *l_label, *lra_label;

  GtkButton* reset_history;

  GtkComboBoxText* reference;

  GtkToggleButton* bypass;

  GSettings* settings;

  std::shared_ptr<AutoGain> autogain;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

G_DEFINE_TYPE(AutogainBox, autogain_box, GTK_TYPE_BOX)

void on_bypass(AutogainBox* self, GtkToggleButton* btn) {
  self->autogain->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(AutogainBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "target");

  g_settings_reset(self->settings, "reference");
}

void on_reset_history(AutogainBox* self, GtkButton* btn) {
  // it is ugly but will ensure that third party tools are able to reset this plugin history

  g_settings_set_boolean(self->settings, "reset-history", !g_settings_get_boolean(self->settings, "reset-history"));
}

void setup(AutogainBox* self, std::shared_ptr<AutoGain> autogain, const std::string& schema_path) {
  self->autogain = autogain;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.autogain", schema_path.c_str());

  autogain->post_messages = true;
  autogain->bypass = false;

  self->connections.push_back(autogain->results.connect(
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

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "target", gtk_spin_button_get_adjustment(self->target), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(
      self->settings, "reference", self->reference, "active", G_SETTINGS_BIND_DEFAULT,
      +[](GValue* value, GVariant* variant, gpointer user_data) {
        const auto* v = g_variant_get_string(variant, nullptr);

        if (g_strcmp0(v, "Momentary") == 0) {
          g_value_set_int(value, 0);
        } else if (g_strcmp0(v, "Shortterm") == 0) {
          g_value_set_int(value, 1);
        } else if (g_strcmp0(v, "Integrated") == 0) {
          g_value_set_int(value, 2);
        } else if (g_strcmp0(v, "Geometric Mean") == 0) {
          g_value_set_int(value, 3);
        }

        return 1;
      },
      +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
        switch (g_value_get_int(value)) {
          case 0:
            return g_variant_new_string("Momentary");

          case 1:
            return g_variant_new_string("Shortterm");

          case 2:
            return g_variant_new_string("Integrated");

          default:
            return g_variant_new_string("Geometric Mean");
        }
      },
      nullptr, nullptr);
}

void dispose(GObject* object) {
  auto* self = EE_AUTOGAIN_BOX(object);

  self->autogain->post_messages = false;
  self->autogain->bypass = false;

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

  G_OBJECT_CLASS(autogain_box_parent_class)->dispose(object);
}

void autogain_box_class_init(AutogainBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

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

  prepare_spinbutton<"dB">(self->target);
}

auto create() -> AutogainBox* {
  return static_cast<AutogainBox*>(g_object_new(EE_TYPE_AUTOGAIN_BOX, nullptr));
}

}  // namespace ui::autogain_box

AutoGainUi::AutoGainUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& schema,
                       const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::autogain;

  // loading builder widgets

  setup_input_output_gain(builder);
}

AutoGainUi::~AutoGainUi() {
  util::debug(name + " ui destroyed");
}

auto AutoGainUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> AutoGainUi* {
  const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/autogain.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<AutoGainUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.autogain", schema_path + "autogain/");

  stack->add(*ui, plugin_name::autogain);

  return ui;
}

void AutoGainUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("target");

  settings->reset("reference");
}
