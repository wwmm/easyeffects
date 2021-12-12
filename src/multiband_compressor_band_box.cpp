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

#include "multiband_compressor_band_box.hpp"

namespace ui::multiband_compressor_band_box {

using namespace std::string_literals;

auto constexpr log_tag = "multiband_compressor_band_box: ";

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  int index;

  std::vector<gulong> gconnections;
};

struct _MultibandCompressorBandBox {
  GtkBox parent_instance;

  GtkToggleButton *bypass, *mute, *solo;

  GtkLabel *end_label, *gain_label, *envelope_label, *curve_label;

  GtkSpinButton *split_frequency, *lowcut_filter_frequency, *highcut_filter_frequency, *attack_time, *attack_threshold,
      *release_time, *release_threshold, *ratio, *knee, *makeup, *sidechain_preamp, *sidechain_reactivity,
      *sidechain_lookahead, *boost_amount, *boost_threshold;

  GtkCheckButton *lowcut_filter, *highcut_filter;

  GtkComboBoxText* compression_mode;

  GtkBox* split_frequency_box;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(MultibandCompressorBandBox, multiband_compressor_band_box, GTK_TYPE_BOX)

void on_reset_quality(MultibandCompressorBandBox* self, GtkButton* btn) {
  // g_settings_reset(self->settings, tags::multiband_compressor::band_q[self->data->index]);
}

void on_reset_frequency(MultibandCompressorBandBox* self, GtkButton* btn) {
  // g_settings_reset(self->settings, tags::multiband_compressor::band_frequency[self->data->index]);
}

gboolean set_boost_threshold_sensitive(MultibandCompressorBandBox* self, const char* active_id) {
  if (g_strcmp0(active_id, "Downward") == 0 || g_strcmp0(active_id, "Boosting") == 0) {
    return 0;
  } else if (g_strcmp0(active_id, "Upward") == 0) {
    return 1;
  }

  return 1;
}

gboolean set_boost_amount_sensitive(MultibandCompressorBandBox* self, const char* active_id) {
  if (g_strcmp0(active_id, "Downward") == 0 || g_strcmp0(active_id, "Upward") == 0) {
    return 0;
  } else if (g_strcmp0(active_id, "Boosting") == 0) {
    return 1;
  }

  return 1;
}
void setup(MultibandCompressorBandBox* self, GSettings* settings, int index) {
  self->data->index = index;
  self->settings = settings;

  using namespace tags::multiband_compressor;

  g_settings_bind(self->settings, band_compression_mode[index], self->compression_mode, "active-id",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_mute[index], self->mute, "active", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_gain[index],
  //                 gtk_range_get_adjustment(GTK_RANGE(self->band_scale)), "value", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_frequency[index],
  //                 gtk_spin_button_get_adjustment(self->band_frequency), "value", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_q[index],
  //                 gtk_spin_button_get_adjustment(self->band_quality), "value", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_solo[index], self->band_solo, "active",
  //                 G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_mute[index], self->band_mute, "active",
  //                 G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_type[index], self->band_type, "active-id",
  //                 G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_mode[index], self->band_mode, "active-id",
  //                 G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, tags::multiband_compressor::band_slope[index], self->band_slope, "active-id",
  //                 G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BAND_BOX(object);

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->gconnections.clear();

  util::debug(log_tag + "index: "s + std::to_string(self->data->index) + " disposed"s);

  G_OBJECT_CLASS(multiband_compressor_band_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BAND_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(multiband_compressor_band_box_parent_class)->finalize(object);
}

void multiband_compressor_band_box_class_init(MultibandCompressorBandBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class,
                                              "/com/github/wwmm/easyeffects/ui/multiband_compressor_band.ui");

  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, bypass);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, mute);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, solo);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, end_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, gain_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, envelope_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, curve_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, split_frequency);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, split_frequency_box);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, lowcut_filter);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, highcut_filter);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, lowcut_filter_frequency);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, highcut_filter_frequency);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, attack_time);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, attack_threshold);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, release_time);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, release_threshold);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, ratio);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, knee);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, makeup);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, sidechain_preamp);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, sidechain_reactivity);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, sidechain_lookahead);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, boost_amount);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, boost_threshold);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, compression_mode);

  gtk_widget_class_bind_template_callback(widget_class, set_boost_amount_sensitive);
  gtk_widget_class_bind_template_callback(widget_class, set_boost_threshold_sensitive);
}

void multiband_compressor_band_box_init(MultibandCompressorBandBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbutton<"Hz">(self->lowcut_filter_frequency);
  prepare_spinbutton<"Hz">(self->highcut_filter_frequency);
  prepare_spinbutton<"ms">(self->attack_time);
  prepare_spinbutton<"ms">(self->release_time);
  prepare_spinbutton<"ms">(self->sidechain_reactivity);
  prepare_spinbutton<"ms">(self->sidechain_lookahead);
  prepare_spinbutton<"dB">(self->attack_threshold);
  prepare_spinbutton<"dB">(self->release_threshold);
  prepare_spinbutton<"dB">(self->knee);
  prepare_spinbutton<"dB">(self->makeup);
  prepare_spinbutton<"dB">(self->sidechain_preamp);
  prepare_spinbutton<"dB">(self->boost_amount);
  prepare_spinbutton<"dB">(self->boost_threshold);

  prepare_spinbutton<"">(self->ratio);
}

auto create() -> MultibandCompressorBandBox* {
  return static_cast<MultibandCompressorBandBox*>(g_object_new(EE_TYPE_MULTIBAND_COMPRESSOR_BAND_BOX, nullptr));
}

}  // namespace ui::multiband_compressor_band_box