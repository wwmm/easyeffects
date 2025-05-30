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

#include "maximizer_ui.hpp"
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
#include "maximizer.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::maximizer_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0U;

  std::shared_ptr<Maximizer> maximizer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _MaximizerBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label,
      *plugin_credit;

  GtkSpinButton *release, *threshold;

  GtkLevelBar* reduction_levelbar;

  GtkLabel* reduction_label;

  GSettings* settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(MaximizerBox, maximizer_box, GTK_TYPE_BOX)

void on_reset(MaximizerBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void setup(MaximizerBox* self, std::shared_ptr<Maximizer> maximizer, const std::string& schema_path) {
  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->data->maximizer = maximizer;

  self->settings = g_settings_new_with_path(tags::schema::maximizer::id, schema_path.c_str());

  maximizer->set_post_messages(true);

  self->data->connections.push_back(maximizer->input_level.connect([=](const float left, const float right) {
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

  self->data->connections.push_back(maximizer->output_level.connect([=](const float left, const float right) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                       self->output_level_right_label, left, right);
        },
        [=]() { g_object_unref(self); });
  }));

  self->data->connections.push_back(maximizer->reduction.connect([=](const double value) {
    g_object_ref(self);

    util::idle_add(
        [=]() {
          if (get_ignore_filter_idle_add(serial)) {
            return;
          }

          if (!GTK_IS_LEVEL_BAR(self->reduction_levelbar) || !GTK_IS_LABEL(self->reduction_label)) {
            return;
          }

          gtk_level_bar_set_value(self->reduction_levelbar, value);
          gtk_label_set_text(self->reduction_label, fmt::format("{0:.0f}", value).c_str());
        },
        [=]() { g_object_unref(self); });
  }));

  gtk_label_set_text(self->plugin_credit, ui::get_plugin_credit_translated(self->data->maximizer->package).c_str());

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "release", gtk_spin_button_get_adjustment(self->release), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "threshold", gtk_spin_button_get_adjustment(self->threshold), "value",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MAXIMIZER_BOX(object);

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

  G_OBJECT_CLASS(maximizer_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MAXIMIZER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(maximizer_box_parent_class)->finalize(object);
}

void maximizer_box_class_init(MaximizerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::maximizer_ui);

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
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, plugin_credit);

  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, release);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, threshold);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, reduction_levelbar);
  gtk_widget_class_bind_template_child(widget_class, MaximizerBox, reduction_label);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void maximizer_box_init(MaximizerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbutton<"dB">(self->threshold);

  prepare_spinbuttons<"ms">(self->release);
}

auto create() -> MaximizerBox* {
  return static_cast<MaximizerBox*>(g_object_new(EE_TYPE_MAXIMIZER_BOX, nullptr));
}

}  // namespace ui::maximizer_box
