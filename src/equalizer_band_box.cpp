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

#include "equalizer_band_box.hpp"

namespace ui::equalizer_band_box {

using namespace std::string_literals;

auto constexpr log_tag = "equalizer_band_box: ";

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  int index;

  std::vector<gulong> gconnections;

  std::locale user_locale = std::locale(setlocale(LC_ALL, nullptr));
};

struct _EqualizerBandBox {
  GtkBox parent_instance;

  GtkComboBoxText *band_type, *band_mode, *band_slope;

  GtkButton *reset_frequency, *reset_quality;

  GtkToggleButton *band_solo, *band_mute;

  GtkScale* band_scale;

  GtkSpinButton *band_frequency, *band_quality;

  GtkPopover* popover_menu;

  GSettings *settings, *app_settings;

  Data* data;
};

G_DEFINE_TYPE(EqualizerBandBox, equalizer_band_box, GTK_TYPE_BOX)

void on_reset_quality(EqualizerBandBox* self, GtkButton* btn) {
  g_settings_reset(self->settings, tags::equalizer::band_q[self->data->index]);
}

void on_reset_frequency(EqualizerBandBox* self, GtkButton* btn) {
  g_settings_reset(self->settings, tags::equalizer::band_frequency[self->data->index]);
}

auto set_band_label(EqualizerBandBox* self, double value) -> const char* {
  if (self->data == nullptr) {
    return g_strdup("");
  }

  if (value > 1000.0) {
    return g_strdup(fmt::format(self->data->user_locale, "{0:.1Lf} kHz", value / 1000.0).c_str());
  } else {
    return g_strdup(fmt::format(self->data->user_locale, "{0:.1Lf} Hz", value).c_str());
  }
}

auto set_band_quality_label(EqualizerBandBox* self, double value) -> const char* {
  if (self->data == nullptr) {
    return g_strdup("");
  }

  return g_strdup(fmt::format(self->data->user_locale, "Q {0:.2Lf}", value).c_str());
}

auto set_band_width_label(EqualizerBandBox* self, double quality, double frequency) -> const char* {
  if (self->data == nullptr) {
    return g_strdup("");
  }

  if (quality > 0.0) {
    return g_strdup(fmt::format(self->data->user_locale, "{0:.1Lf} Hz", frequency / quality).c_str());
  } else {
    return g_strdup(_("infinity"));
  }
}

auto set_band_scale_sensitive(EqualizerBandBox* self, const char* active_id) -> gboolean {
  if (g_strcmp0(active_id, "Off") == 0 || g_strcmp0(active_id, "Hi-pass") == 0 ||
      g_strcmp0(active_id, "Lo-pass") == 0) {
    return 0;
  }

  return 1;
}

void setup(EqualizerBandBox* self, GSettings* settings, int index) {
  self->data->index = index;
  self->settings = settings;

  g_settings_bind(settings, tags::equalizer::band_gain[index], gtk_range_get_adjustment(GTK_RANGE(self->band_scale)),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_frequency[index],
                  gtk_spin_button_get_adjustment(self->band_frequency), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_q[index], gtk_spin_button_get_adjustment(self->band_quality), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_solo[index], self->band_solo, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_mute[index], self->band_mute, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_type[index], self->band_type, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_mode[index], self->band_mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, tags::equalizer::band_slope[index], self->band_slope, "active-id", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_EQUALIZER_BAND_BOX(object);

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->gconnections.clear();

  g_object_unref(self->app_settings);

  util::debug(log_tag + "index: "s + util::to_string(self->data->index) + " disposed"s);

  G_OBJECT_CLASS(equalizer_band_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_EQUALIZER_BAND_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(equalizer_band_box_parent_class)->finalize(object);
}

void equalizer_band_box_class_init(EqualizerBandBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/equalizer_band.ui");

  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_type);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_mode);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_slope);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, reset_frequency);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, reset_quality);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_solo);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_mute);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_scale);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_frequency);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, band_quality);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBandBox, popover_menu);

  gtk_widget_class_bind_template_callback(widget_class, on_reset_quality);
  gtk_widget_class_bind_template_callback(widget_class, on_reset_frequency);
  gtk_widget_class_bind_template_callback(widget_class, set_band_scale_sensitive);
  gtk_widget_class_bind_template_callback(widget_class, set_band_label);
  gtk_widget_class_bind_template_callback(widget_class, set_band_quality_label);
  gtk_widget_class_bind_template_callback(widget_class, set_band_width_label);
}

void equalizer_band_box_init(EqualizerBandBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  self->app_settings = g_settings_new("com.github.wwmm.easyeffects");

  g_settings_bind(self->app_settings, "autohide-popovers", self->popover_menu, "autohide", G_SETTINGS_BIND_DEFAULT);

  prepare_scales<"">(self->band_scale);

  prepare_spinbuttons<"Hz">(self->band_frequency);
  prepare_spinbuttons<"">(self->band_quality);
}

auto create() -> EqualizerBandBox* {
  return static_cast<EqualizerBandBox*>(g_object_new(EE_TYPE_EQUALIZER_BAND_BOX, nullptr));
}

}  // namespace ui::equalizer_band_box
