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

#include "multiband_gate_band_box.hpp"

namespace ui::multiband_gate_band_box {

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  int index;

  std::vector<gulong> gconnections;
};

struct _MultibandGateBandBox {
  GtkBox parent_instance;

  GtkToggleButton *bypass, *mute, *solo, *external_sidechain, *hysteresis;

  GtkLabel *end_label, *gain_label, *envelope_label, *curve_label, *gating_label;

  GtkSpinButton *split_frequency, *lowcut_filter_frequency, *highcut_filter_frequency, *attack_time, *release_time,
      *hysteresis_threshold, *hysteresis_zone, *curve_threshold, *curve_zone, *reduction, *makeup, *sidechain_preamp,
      *sidechain_reactivity, *sidechain_lookahead;

  GtkCheckButton *lowcut_filter, *highcut_filter;

  GtkComboBoxText *sidechain_mode, *sidechain_source;

  GtkBox* split_frequency_box;

  GtkLevelBar* gating;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(MultibandGateBandBox, multiband_gate_band_box, GTK_TYPE_BOX)

void set_end_label(MultibandGateBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->end_label)) {
    return;
  }

  gtk_label_set_text(self->end_label, fmt::format("{0:.0f}", value).c_str());
}

void set_envelope_label(MultibandGateBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->envelope_label)) {
    return;
  }

  gtk_label_set_text(self->envelope_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
}

void set_curve_label(MultibandGateBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->curve_label)) {
    return;
  }

  gtk_label_set_text(self->curve_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
}

void set_gain_label(MultibandGateBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LABEL(self->gain_label)) {
    return;
  }

  gtk_label_set_text(self->gating_label, fmt::format("{0:.0Lf}", util::linear_to_db(value)).c_str());
  gtk_label_set_text(self->gain_label, fmt::format("{0:.0f}", util::linear_to_db(value)).c_str());
}

void set_gating_levelbar(MultibandGateBandBox* self, const float& value) {
  if (!GTK_IS_WIDGET(self)) {
    return;
  }

  if (!GTK_IS_LEVEL_BAR(self->gating)) {
    return;
  }

  gtk_level_bar_set_value(self->gating, value);
}

void setup(MultibandGateBandBox* self, GSettings* settings, int index) {
  self->data->index = index;
  self->settings = settings;

  using namespace tags::multiband_gate;

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

  g_settings_bind(self->settings, band_gate_enable[index].data(), self->bypass, "active",
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

  g_settings_bind(settings, band_release_time[index].data(), gtk_spin_button_get_adjustment(self->release_time),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, band_hysteresis[index].data(), self->hysteresis, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_hysteresis_threshold[index].data(),
                  gtk_spin_button_get_adjustment(self->hysteresis_threshold), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_hysteresis_zone[index].data(), gtk_spin_button_get_adjustment(self->hysteresis_zone),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_curve_threshold[index].data(), gtk_spin_button_get_adjustment(self->curve_threshold),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_curve_zone[index].data(), gtk_spin_button_get_adjustment(self->curve_zone), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_reduction[index].data(), gtk_spin_button_get_adjustment(self->reduction), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_makeup[index].data(), gtk_spin_button_get_adjustment(self->makeup), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_sidechain_preamp[index].data(), gtk_spin_button_get_adjustment(self->sidechain_preamp),
                  "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_sidechain_reactivity[index].data(),
                  gtk_spin_button_get_adjustment(self->sidechain_reactivity), "value", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, band_sidechain_lookahead[index].data(),
                  gtk_spin_button_get_adjustment(self->sidechain_lookahead), "value", G_SETTINGS_BIND_DEFAULT);
}

void dispose(GObject* object) {
  auto* self = EE_MULTIBAND_GATE_BAND_BOX(object);

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->gconnections.clear();

  util::debug("index: " + util::to_string(self->data->index) + " disposed");

  G_OBJECT_CLASS(multiband_gate_band_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_MULTIBAND_GATE_BAND_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(multiband_gate_band_box_parent_class)->finalize(object);
}

void multiband_gate_band_box_class_init(MultibandGateBandBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::multiband_gate_band_ui);

  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, bypass);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, mute);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, solo);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, external_sidechain);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, end_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, gain_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, gating);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, gating_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, envelope_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, curve_label);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, split_frequency);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, split_frequency_box);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, lowcut_filter);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, highcut_filter);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, lowcut_filter_frequency);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, highcut_filter_frequency);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, attack_time);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, release_time);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, hysteresis);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, hysteresis_threshold);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, hysteresis_zone);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, curve_threshold);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, curve_zone);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, reduction);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, makeup);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, sidechain_preamp);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, sidechain_reactivity);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, sidechain_lookahead);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, sidechain_mode);
  gtk_widget_class_bind_template_child(widget_class, MultibandGateBandBox, sidechain_source);
}

void multiband_gate_band_box_init(MultibandGateBandBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbuttons<"Hz">(self->lowcut_filter_frequency, self->highcut_filter_frequency, self->split_frequency);

  prepare_spinbuttons<"ms">(self->attack_time, self->release_time, self->sidechain_reactivity,
                            self->sidechain_lookahead);

  prepare_spinbuttons<"dB">(self->hysteresis_threshold, self->hysteresis_zone, self->curve_threshold, self->curve_zone,
                            self->reduction, self->makeup, self->sidechain_preamp);
}

auto create() -> MultibandGateBandBox* {
  return static_cast<MultibandGateBandBox*>(g_object_new(EE_TYPE_MULTIBAND_GATE_BAND_BOX, nullptr));
}

}  // namespace ui::multiband_gate_band_box
