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

#include "equalizer_band_box.hpp"
#include <adwaita.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include "tags_app.hpp"
#include "tags_equalizer.hpp"
#include "tags_resources.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::equalizer_band_box {

using namespace tags::equalizer;

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  int index = 0;  // index in the gsettings database
};

struct _EqualizerBandBox {
  GtkBox parent_instance;

  AdwComboRow *band_type, *band_mode, *band_slope;

  GtkButton *reset_frequency, *reset_quality;

  GtkSwitch *band_solo, *band_mute;

  GtkScale* band_scale;

  GtkSpinButton *band_frequency, *band_gain, *band_quality, *band_width;

  GtkPopover* popover_menu;

  GtkLabel* band_number_label;

  GSettings *settings, *app_settings;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(EqualizerBandBox, equalizer_band_box, GTK_TYPE_BOX)

void on_reset_frequency(EqualizerBandBox* self, GtkButton* btn) {
  g_settings_reset(self->settings, band_frequency[self->data->index].data());
}

void on_reset_gain(EqualizerBandBox* self, GtkButton* btn) {
  g_settings_reset(self->settings, band_gain[self->data->index].data());
}

void on_reset_quality(EqualizerBandBox* self, GtkButton* btn) {
  g_settings_reset(self->settings, band_q[self->data->index].data());
}

void on_reset_width(EqualizerBandBox* self, GtkButton* btn) {
  g_settings_reset(self->settings, band_width[self->data->index].data());
}

auto set_band_label(EqualizerBandBox* self, double value) -> const char* {
  if (value < 1000.0) {
    // Show no decimal digits: full integer. No need of locale.
    return g_strdup(fmt::format("{0:.0f} Hz", value).c_str());
  }

  // Convert in kHz and show hHz as 1 decimal digit. Use locale.
  return g_strdup(fmt::format(ui::get_user_locale(), "{0:.1Lf} kHz", value / 1000.0).c_str());
}

auto set_band_quality_label(EqualizerBandBox* self, double value) -> const char* {
  return g_strdup(fmt::format(ui::get_user_locale(), "Q {0:.2Lf}", value).c_str());
}

auto set_band_gain_sensitive(EqualizerBandBox* self, const guint selected_id) -> gboolean {
  switch (selected_id) {
    case 0U:  // Off
    case 2U:  // High Pass
    case 4U:  // Low Pass
      return 0;

    default:
      break;
  }

  return 1;
}

auto set_band_width_sensitive(EqualizerBandBox* self, const guint selected_id) -> gboolean {
  switch (selected_id) {
    case 9U:   // Band Pass
    case 10U:  // Ladder Pass
    case 11U:  // Ladder Rej
      return 1;

    default:
      break;
  }

  return 0;
}

void setup(EqualizerBandBox* self, GSettings* settings) {
  self->settings = settings;
}

void bind(EqualizerBandBox* self, int index) {
  self->data->index = index;

  gtk_label_set_text(self->band_number_label, util::to_string(index + 1).c_str());

  g_settings_bind(self->settings, band_gain[index].data(), gtk_range_get_adjustment(GTK_RANGE(self->band_scale)),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_frequency[index].data(), gtk_spin_button_get_adjustment(self->band_frequency),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_q[index].data(), gtk_spin_button_get_adjustment(self->band_quality), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_width[index].data(), gtk_spin_button_get_adjustment(self->band_width), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_solo[index].data(), self->band_solo, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_mute[index].data(), self->band_mute, "active", G_SETTINGS_BIND_DEFAULT);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, band_type[index].data(), self->band_type);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, band_mode[index].data(), self->band_mode);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, band_slope[index].data(), self->band_slope);
}

void dispose(GObject* object) {
  auto* self = EE_EQUALIZER_BAND_BOX(object);

  g_object_unref(self->app_settings);

  util::debug("index: " + util::to_string(self->data->index) + " disposed");

  G_OBJECT_CLASS(equalizer_band_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_EQUALIZER_BAND_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(equalizer_band_box_parent_class)->finalize(object);
}

void equalizer_band_box_class_init(EqualizerBandBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::equalizer_band_ui);

  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_type);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_mode);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_slope);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, reset_frequency);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, reset_quality);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_solo);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_mute);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_scale);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_frequency);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_gain);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_quality);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_width);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, popover_menu);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_number_label);

  gtk_widget_class_bind_template_callback(widget_class, on_reset_frequency);
  gtk_widget_class_bind_template_callback(widget_class, on_reset_gain);
  gtk_widget_class_bind_template_callback(widget_class, on_reset_quality);
  gtk_widget_class_bind_template_callback(widget_class, on_reset_width);
  gtk_widget_class_bind_template_callback(widget_class, set_band_gain_sensitive);
  gtk_widget_class_bind_template_callback(widget_class, set_band_width_sensitive);
  gtk_widget_class_bind_template_callback(widget_class, set_band_label);
  gtk_widget_class_bind_template_callback(widget_class, set_band_quality_label);
}

void equalizer_band_box_init(EqualizerBandBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->app_settings = g_settings_new(tags::app::id);

  g_settings_bind(self->app_settings, "autohide-popovers", self->popover_menu, "autohide", G_SETTINGS_BIND_DEFAULT);

  prepare_scales<"">(self->band_scale);

  prepare_spinbuttons<"Hz">(self->band_frequency);
  prepare_spinbuttons<"dB">(self->band_gain);
  prepare_spinbuttons<"">(self->band_quality);
  prepare_spinbuttons<"oct">(self->band_width);
}

auto create() -> EqualizerBandBox* {
  return static_cast<EqualizerBandBox*>(g_object_new(EE_TYPE_EQUALIZER_BAND_BOX, nullptr));
}

}  // namespace ui::equalizer_band_box
