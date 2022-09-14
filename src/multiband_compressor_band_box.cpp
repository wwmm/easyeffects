/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include "multiband_compressor_band_box.hpp"

namespace ui::multiband_compressor_band_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  int index;

  std::vector<gulong> gconnections;
};

struct _MultibandCompressorBandBox {
  GtkBox parent_instance;

  GtkToggleButton *bypass, *mute, *solo, *external_sidechain;

  GtkLabel *end_label, *gain_label, *envelope_label, *curve_label;

  GtkSpinButton *split_frequency, *lowcut_filter_frequency, *highcut_filter_frequency, *attack_time, *attack_threshold,
      *release_time, *release_threshold, *ratio, *knee, *makeup, *sidechain_preamp, *sidechain_reactivity,
      *sidechain_lookahead, *boost_amount, *boost_threshold;

  GtkCheckButton *lowcut_filter, *highcut_filter;

  GtkComboBoxText *compression_mode, *sidechain_mode, *sidechain_source;

  GtkBox* split_frequency_box;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(MultibandCompressorBandBox, multiband_compressor_band_box, GTK_TYPE_BOX)

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

void set_end_label(MultibandCompressorBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->end_label)) {
    return;
  }

  gtk_label_set_text(self->end_label, fmt::format("{0:.0f}", value).c_str());
}

void set_envelope_label(MultibandCompressorBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->envelope_label)) {
    return;
  }

  gtk_label_set_text(self->envelope_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
}

void set_curve_label(MultibandCompressorBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->curve_label)) {
    return;
  }

  gtk_label_set_text(self->curve_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
}

void set_gain_label(MultibandCompressorBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->gain_label)) {
    return;
  }

  gtk_label_set_text(self->gain_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
}

void setup(MultibandCompressorBandBox* self, GSettings* settings, int index) {
  self->data->index = index;
  self->settings = settings;

  using namespace tags::multiband_compressor;

  if (index > 0) {
    g_settings_bind(settings, band_split_frequency[index].data(), gtk_spin_button_get_adjustment(self->split_frequency),
                    "value", G_SETTINGS_BIND_DEFAULT);
  } else {
    // removing split frequency from band 0

    for (auto* child = gtk_widget_get_last_child(GTK_WIDGET(self->split_frequency_box)); child != nullptr;
         child = gtk_widget_get_last_child(GTK_WIDGET(self->split_frequency_box))) {
      gtk_box_remove(self->split_frequency_box, child);
    }

    auto* label = gtk_label_new("0 Hz");

    gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_CENTER);

    gtk_box_append(self->split_frequency_box, GTK_WIDGET(label));
  }

  g_settings_bind(self->settings, band_compression_mode[index].data(), self->compression_mode, "active-id",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_compressor_enable[index].data(), self->bypass, "active",
                  G_SETTINGS_BIND_INVERT_BOOLEAN);

  g_settings_bind(self->settings, band_mute[index].data(), self->mute, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_solo[index].data(), self->solo, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_lowcut_filter[index].data(), self->lowcut_filter, "active",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_highcut_filter[index].data(), self->highcut_filter, "active",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_external_sidechain[index].data(), self->external_sidechain, "active",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_sidechain_mode[index].data(), self->sidechain_mode, "active-id",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_sidechain_source[index].data(), self->sidechain_source, "active-id",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_lowcut_filter_frequency[index].data(),
                  gtk_spin_button_get_adjustment(self->lowcut_filter_frequency), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_highcut_filter_frequency[index].data(),
                  gtk_spin_button_get_adjustment(self->highcut_filter_frequency), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_attack_time[index].data(), gtk_spin_button_get_adjustment(self->attack_time), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_attack_threshold[index].data(), gtk_spin_button_get_adjustment(self->attack_threshold),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_release_time[index].data(), gtk_spin_button_get_adjustment(self->release_time),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_release_threshold[index].data(),
                  gtk_spin_button_get_adjustment(self->release_threshold), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_ratio[index].data(), gtk_spin_button_get_adjustment(self->ratio), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_knee[index].data(), gtk_spin_button_get_adjustment(self->knee), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_makeup[index].data(), gtk_spin_button_get_adjustment(self->makeup), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_sidechain_preamp[index].data(), gtk_spin_button_get_adjustment(self->sidechain_preamp),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_sidechain_reactivity[index].data(),
                  gtk_spin_button_get_adjustment(self->sidechain_reactivity), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_sidechain_lookahead[index].data(),
                  gtk_spin_button_get_adjustment(self->sidechain_lookahead), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_boost_amount[index].data(), gtk_spin_button_get_adjustment(self->boost_amount),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_boost_threshold[index].data(), gtk_spin_button_get_adjustment(self->boost_threshold),
                  "value", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BAND_BOX(object);

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->gconnections.clear();

  util::debug("index: " + util::to_string(self->data->index) + " disposed");

  G_OBJECT_CLASS(multiband_compressor_band_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MULTIBAND_COMPRESSOR_BAND_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(multiband_compressor_band_box_parent_class)->finalize(object);
}

void multiband_compressor_band_box_class_init(MultibandCompressorBandBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::multiband_compressor_band_ui);

  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, bypass);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, mute);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, solo);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, external_sidechain);
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
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, sidechain_mode);
  gtk_widget_class_bind_template_child(widget_class, MultibandCompressorBandBox, sidechain_source);

  gtk_widget_class_bind_template_callback(widget_class, set_boost_amount_sensitive);
  gtk_widget_class_bind_template_callback(widget_class, set_boost_threshold_sensitive);
}

void multiband_compressor_band_box_init(MultibandCompressorBandBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbuttons<"Hz">(self->lowcut_filter_frequency, self->highcut_filter_frequency, self->split_frequency);

  prepare_spinbuttons<"ms">(self->attack_time, self->release_time, self->sidechain_reactivity,
                            self->sidechain_lookahead);

  prepare_spinbuttons<"dB">(self->attack_threshold, self->knee, self->makeup, self->sidechain_preamp,
                            self->boost_amount, self->boost_threshold);

  prepare_spinbuttons<"">(self->ratio);

  // This spinbutton can assume -inf
  prepare_spinbuttons<"dB", false>(self->release_threshold);
}

auto create() -> MultibandCompressorBandBox* {
  return static_cast<MultibandCompressorBandBox*>(g_object_new(EE_TYPE_MULTIBAND_COMPRESSOR_BAND_BOX, nullptr));
}

}  // namespace ui::multiband_compressor_band_box
