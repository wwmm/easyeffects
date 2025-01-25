/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include "level_meter_ui.hpp"
#include <STTypes.h>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glibconfig.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <sigc++/connection.h>
#include <memory>
#include <string>
#include <vector>
#include "level_meter.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::level_meter_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0U;

  std::shared_ptr<LevelMeter> level_meter;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _LevelMeterBox {
  GtkBox parent_instance;

  GtkLevelBar *input_level_left, *input_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *plugin_credit;

  GtkLevelBar *m_level, *s_level, *i_level, *r_level, *lra_level;

  GtkLabel *m_label, *s_label, *i_label, *r_label, *lra_label, *true_peak_left_label, *true_peak_right_label;

  GtkButton* reset_history;

  GSettings* settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(LevelMeterBox, level_meter_box, GTK_TYPE_BOX)

void on_reset_history(LevelMeterBox* self, GtkButton* btn) {
  // Since there's no reason why someone would want to activate the reset-history
  // through a third party tool, we do not bind this action to a gsettings key
  // like it's done in the AutoGain.

  self->data->level_meter->reset_history();
}

void setup(LevelMeterBox* self, std::shared_ptr<LevelMeter> level_meter, const std::string& schema_path) {
  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->data->level_meter = level_meter;

  self->settings = g_settings_new_with_path(tags::schema::level_meter::id, schema_path.c_str());

  level_meter->set_post_messages(true);

  self->data->connections.push_back(level_meter->input_level.connect([=](const float left, const float right) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                       self->input_level_right_label, left, right);
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(level_meter->results.connect(
      [=](const double momentary, const double shortterm, const double integrated, const double relative,
          const double range, const double true_peak_L, const double true_peak_R) {
        g_object_ref(self);

        util::idle_add(
            [=]() {
              if (get_ignore_filter_idle_add(serial)) {
                return;
              }

              if (!GTK_IS_LEVEL_BAR(self->m_level) || !GTK_IS_LABEL(self->m_label) ||
                  !GTK_IS_LEVEL_BAR(self->s_level) || !GTK_IS_LABEL(self->s_label) ||
                  !GTK_IS_LEVEL_BAR(self->i_level) || !GTK_IS_LABEL(self->i_label) ||
                  !GTK_IS_LEVEL_BAR(self->r_level) || !GTK_IS_LABEL(self->r_label) ||
                  !GTK_IS_LEVEL_BAR(self->lra_level) || !GTK_IS_LABEL(self->lra_label) ||
                  !GTK_IS_LABEL(self->true_peak_left_label) || !GTK_IS_LABEL(self->true_peak_right_label)) {
                return;
              }

              gtk_label_set_text(self->true_peak_left_label,
                                 fmt::format("{0:.0f} dB", util::linear_to_db(true_peak_L)).c_str());
              gtk_label_set_text(self->true_peak_right_label,
                                 fmt::format("{0:.0f} dB", util::linear_to_db(true_peak_R)).c_str());

              gtk_level_bar_set_value(self->m_level, util::db_to_linear(momentary));
              gtk_label_set_text(self->m_label, fmt::format("{0:.0f} LUFS", momentary).c_str());

              gtk_level_bar_set_value(self->s_level, util::db_to_linear(shortterm));
              gtk_label_set_text(self->s_label, fmt::format("{0:.0f} LUFS", shortterm).c_str());

              gtk_level_bar_set_value(self->i_level, util::db_to_linear(integrated));
              gtk_label_set_text(self->i_label, fmt::format("{0:.0f} LUFS", integrated).c_str());

              gtk_level_bar_set_value(self->r_level, util::db_to_linear(relative));
              gtk_label_set_text(self->r_label, fmt::format("{0:.0f} LUFS", relative).c_str());

              gtk_level_bar_set_value(self->lra_level, util::db_to_linear(range));
              gtk_label_set_text(self->lra_label, fmt::format("{0:.0f} LU", range).c_str());
            },
            [=]() { g_object_unref(self); });
      }));

  gtk_label_set_text(self->plugin_credit, ui::get_plugin_credit_translated(self->data->level_meter->package).c_str());
}

void dispose(GObject* object) {
  auto* self = EE_LEVEL_METER_BOX(object);

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

  G_OBJECT_CLASS(level_meter_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_LEVEL_METER_BOX(object);

  delete self->data;

  util::debug("finalize");

  G_OBJECT_CLASS(level_meter_box_parent_class)->finalize(object);
}

void level_meter_box_class_init(LevelMeterBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::level_meter_ui);

  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, plugin_credit);

  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, reset_history);

  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, m_level);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, s_level);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, i_level);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, r_level);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, lra_level);

  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, m_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, s_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, i_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, r_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, lra_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, true_peak_left_label);
  gtk_widget_class_bind_template_child(widget_class, LevelMeterBox, true_peak_right_label);

  gtk_widget_class_bind_template_callback(widget_class, on_reset_history);
}

void level_meter_box_init(LevelMeterBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();
}

auto create() -> LevelMeterBox* {
  return static_cast<LevelMeterBox*>(g_object_new(EE_TYPE_LEVEL_METER_BOX, nullptr));
}

}  // namespace ui::level_meter_box
