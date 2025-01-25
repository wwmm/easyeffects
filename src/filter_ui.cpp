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

#include "filter_ui.hpp"
#include <STTypes.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glibconfig.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <gtk/gtkdropdown.h>
#include <sigc++/connection.h>
#include <memory>
#include <string>
#include <vector>
#include "filter.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::filter_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0U;

  std::shared_ptr<Filter> filter;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _FilterBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label,
      *plugin_credit;

  GtkDropDown *mode, *equalizer_mode, *type, *slope;

  GtkSpinButton *frequency, *width, *gain, *quality, *balance;

  GtkToggleButton* show_native_ui;

  GSettings* settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(FilterBox, filter_box, GTK_TYPE_BOX)

void on_reset(FilterBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
}

void on_show_native_window(FilterBox* self, GtkToggleButton* btn) {
  if (gtk_toggle_button_get_active(btn) != 0) {
    self->data->filter->show_native_ui();
  } else {
    self->data->filter->close_native_ui();
  }
}

void setup(FilterBox* self, std::shared_ptr<Filter> filter, const std::string& schema_path) {
  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->data->filter = filter;

  self->settings = g_settings_new_with_path(tags::schema::filter::id, schema_path.c_str());

  filter->set_post_messages(true);

  self->data->connections.push_back(filter->input_level.connect([=](const float left, const float right) {
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

  self->data->connections.push_back(filter->output_level.connect([=](const float left, const float right) {
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

  gtk_label_set_text(self->plugin_credit, ui::get_plugin_credit_translated(self->data->filter->package).c_str());

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "frequency", gtk_spin_button_get_adjustment(self->frequency), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "width", gtk_spin_button_get_adjustment(self->width), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "gain", gtk_spin_button_get_adjustment(self->gain), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "quality", gtk_spin_button_get_adjustment(self->quality), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "balance", gtk_spin_button_get_adjustment(self->balance), "value",
                  G_SETTINGS_BIND_DEFAULT);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "type", self->type);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "mode", self->mode);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "equal-mode", self->equalizer_mode);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "slope", self->slope);

  g_settings_bind(ui::get_global_app_settings(), "show-native-plugin-ui", self->show_native_ui, "visible",
                  G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_FILTER_BOX(object);

  self->data->filter->close_native_ui();

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

  G_OBJECT_CLASS(filter_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_FILTER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(filter_box_parent_class)->finalize(object);
}

void filter_box_class_init(FilterBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::filter_ui);

  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, output_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, plugin_credit);

  gtk_widget_class_bind_template_child(widget_class, FilterBox, type);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, mode);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, equalizer_mode);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, slope);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, frequency);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, width);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, gain);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, quality);
  gtk_widget_class_bind_template_child(widget_class, FilterBox, balance);

  gtk_widget_class_bind_template_child(widget_class, FilterBox, show_native_ui);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);
  gtk_widget_class_bind_template_callback(widget_class, on_show_native_window);
}

void filter_box_init(FilterBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);

  prepare_spinbuttons<"Hz">(self->frequency);
  prepare_spinbuttons<"dB">(self->gain);
  prepare_spinbuttons<"">(self->width, self->quality);
  prepare_spinbuttons<"%">(self->balance);
}

auto create() -> FilterBox* {
  return static_cast<FilterBox*>(g_object_new(EE_TYPE_FILTER_BOX, nullptr));
}

}  // namespace ui::filter_box
