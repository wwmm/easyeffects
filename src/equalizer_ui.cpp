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

#include "equalizer_ui.hpp"

namespace ui::equalizer_box {

using namespace std::string_literals;

auto constexpr log_tag = "equalizer_box: ";

constexpr int max_bands = 32U;

enum Channel { left, right };

enum FilterType : const unsigned int {
  PEAKING = 1U,
  LOW_PASS = 1U << 1U,
  LOW_PASS_Q = 1U << 2U,
  HIGH_PASS = 1U << 3U,
  HIGH_PASS_Q = 1U << 4U,
  BAND_PASS = 1U << 5U,
  LOW_SHELF = 1U << 6U,
  LOW_SHELF_xdB = 1U << 7U,
  HIGH_SHELF = 1U << 8U,
  HIGH_SHELF_xdB = 1U << 9U,
  NOTCH = 1U << 10U,
  ALL_PASS = 1U << 11U
};

struct ImportedBand {
  unsigned int type;
  float freq;
  float gain;
  float quality_factor;
  float slope_dB;
};

static std::unordered_map<std::string, FilterType> const FilterTypeMap = {
    {"PK", FilterType::PEAKING},         {"LP", FilterType::LOW_PASS},       {"LPQ", FilterType::LOW_PASS_Q},
    {"HP", FilterType::HIGH_PASS},       {"HPQ", FilterType::HIGH_PASS_Q},   {"BP", FilterType::BAND_PASS},
    {"LS", FilterType::LOW_SHELF},       {"LSC", FilterType::LOW_SHELF_xdB}, {"HS", FilterType::HIGH_SHELF},
    {"HSC", FilterType::HIGH_SHELF_xdB}, {"NO", FilterType::NOTCH},          {"AP", FilterType::ALL_PASS}};

struct _EqualizerBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkStack* stack;

  GtkBox *bands_box_left, *bands_box_right;

  GtkSpinButton* nbands;

  GtkComboBoxText* mode;

  GtkToggleButton* split_channels;

  GSettings *settings, *settings_left, *settings_right;

  std::shared_ptr<Equalizer> equalizer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections, gconnections_left, gconnections_right;
};

G_DEFINE_TYPE(EqualizerBox, equalizer_box, GTK_TYPE_BOX)

void on_bypass(EqualizerBox* self, GtkToggleButton* btn) {
  self->equalizer->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(EqualizerBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  g_settings_reset(self->settings, "input-gain");

  g_settings_reset(self->settings, "output-gain");

  g_settings_reset(self->settings, "mode");
  g_settings_reset(self->settings, "num-bands");
  g_settings_reset(self->settings, "split-channels");

  using namespace tags::equalizer;

  for (int n = 0; n < max_bands; n++) {
    // left channel

    g_settings_reset(self->settings_left, band_gain[n]);
    g_settings_reset(self->settings_left, band_frequency[n]);
    g_settings_reset(self->settings_left, band_q[n]);
    g_settings_reset(self->settings_left, band_type[n]);
    g_settings_reset(self->settings_left, band_mode[n]);
    g_settings_reset(self->settings_left, band_slope[n]);
    g_settings_reset(self->settings_left, band_solo[n]);
    g_settings_reset(self->settings_left, band_mute[n]);

    // right channel

    g_settings_reset(self->settings_right, band_gain[n]);
    g_settings_reset(self->settings_right, band_frequency[n]);
    g_settings_reset(self->settings_right, band_q[n]);
    g_settings_reset(self->settings_right, band_type[n]);
    g_settings_reset(self->settings_right, band_mode[n]);
    g_settings_reset(self->settings_right, band_slope[n]);
    g_settings_reset(self->settings_right, band_solo[n]);
    g_settings_reset(self->settings_right, band_mute[n]);
  }
}

void on_update_quality_width(GtkSpinButton* band_frequency,
                             GtkSpinButton* band_quality,
                             GtkLabel* band_quality_label,
                             GtkLabel* band_width) {
  const auto q = gtk_spin_button_get_value(band_quality);

  gtk_label_set_text(band_quality_label, fmt::format("Q {0:.2f}", q).c_str());

  if (q > 0.0) {
    const auto f = gtk_spin_button_get_value(band_frequency);
    gtk_label_set_text(band_width, fmt::format("{0:.1f} Hz", f / q).c_str());
  } else {
    gtk_label_set_text(band_width, _("infinity"));
  }
}

template <Channel channel>
void build_channel_bands(EqualizerBox* self, const int& nbands, const bool& split_mode) {
  for (int n = 0; n < nbands; n++) {
    auto bandn = const_cast<char*>(tags::equalizer::band_id[n]);

    auto* builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/equalizer_band.ui");

    auto* band_box = GTK_BOX(gtk_builder_get_object(builder, "band_box"));

    auto* band_type = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "band_type"));
    auto* band_mode = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "band_mode"));
    auto* band_slope = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "band_slope"));

    auto* band_label = GTK_LABEL(gtk_builder_get_object(builder, "band_label"));
    auto* band_width = GTK_LABEL(gtk_builder_get_object(builder, "band_width"));
    auto* band_quality_label = GTK_LABEL(gtk_builder_get_object(builder, "band_quality_label"));

    auto* reset_frequency = GTK_BUTTON(gtk_builder_get_object(builder, "reset_frequency"));
    auto* reset_quality = GTK_BUTTON(gtk_builder_get_object(builder, "reset_quality"));

    auto* band_solo = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "band_solo"));
    auto* band_mute = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "band_mute"));

    auto* band_scale = GTK_SCALE(gtk_builder_get_object(builder, "band_scale"));

    auto* band_frequency = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "band_frequency"));
    auto* band_quality = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "band_quality"));

    prepare_scale<"">(band_scale);

    prepare_spinbutton<"Hz">(band_frequency);
    prepare_spinbutton<"">(band_quality);

    g_object_set_data(G_OBJECT(reset_frequency), "bandn", bandn);
    g_object_set_data(G_OBJECT(reset_quality), "bandn", bandn);

    g_object_set_data(G_OBJECT(band_type), "band-scale", band_scale);

    g_object_set_data(G_OBJECT(band_frequency), "band-quality", band_quality);
    g_object_set_data(G_OBJECT(band_frequency), "band-quality-label", band_quality_label);
    g_object_set_data(G_OBJECT(band_frequency), "band-width", band_width);
    g_object_set_data(G_OBJECT(band_frequency), "band-label", band_label);

    g_object_set_data(G_OBJECT(band_quality), "band-frequency", band_quality);
    g_object_set_data(G_OBJECT(band_quality), "band-quality-label", band_quality_label);
    g_object_set_data(G_OBJECT(band_quality), "band-width", band_width);

    g_signal_connect(band_frequency, "value-changed", G_CALLBACK(+[](GtkSpinButton* btn, EqualizerBox* self) {
                       auto* band_quality = GTK_SPIN_BUTTON(g_object_get_data(G_OBJECT(btn), "band-quality"));
                       auto* band_quality_label = GTK_LABEL(g_object_get_data(G_OBJECT(btn), "band-quality-label"));
                       auto* band_width = GTK_LABEL(g_object_get_data(G_OBJECT(btn), "band-width"));
                       auto* band_label = GTK_LABEL(g_object_get_data(G_OBJECT(btn), "band-label"));

                       on_update_quality_width(btn, band_quality, band_quality_label, band_width);

                       if (const auto f = gtk_spin_button_get_value(btn); f > 1000.0) {
                         gtk_label_set_text(band_label, fmt::format("{0:.1f} kHz", f / 1000.0).c_str());
                       } else {
                         gtk_label_set_text(band_label, fmt::format("{0:.0f} Hz", f).c_str());
                       }
                     }),
                     self);

    g_signal_connect(band_quality, "value-changed", G_CALLBACK(+[](GtkSpinButton* btn, EqualizerBox* self) {
                       auto* band_frequency = GTK_SPIN_BUTTON(g_object_get_data(G_OBJECT(btn), "band-frequency"));
                       auto* band_quality_label = GTK_LABEL(g_object_get_data(G_OBJECT(btn), "band-quality-label"));
                       auto* band_width = GTK_LABEL(g_object_get_data(G_OBJECT(btn), "band-width"));

                       on_update_quality_width(band_frequency, btn, band_quality_label, band_width);
                     }),
                     self);

    if (split_mode) {
      g_signal_connect(reset_frequency, "clicked", G_CALLBACK(+[](GtkButton* btn, EqualizerBox* self) {
                         auto bandn = static_cast<const char*>(g_object_get_data(G_OBJECT(btn), "bandn"));

                         if constexpr (channel == Channel::left) {
                           g_settings_reset(self->settings_left, (bandn + "-frequency"s).c_str());
                         } else if constexpr (channel == Channel::right) {
                           g_settings_reset(self->settings_right, (bandn + "-frequency"s).c_str());
                         }
                       }),
                       self);

      g_signal_connect(reset_quality, "clicked", G_CALLBACK(+[](GtkButton* btn, EqualizerBox* self) {
                         auto bandn = static_cast<const char*>(g_object_get_data(G_OBJECT(btn), "bandn"));

                         if constexpr (channel == Channel::left) {
                           g_settings_reset(self->settings_left, (bandn + "-q"s).c_str());
                         } else if constexpr (channel == Channel::right) {
                           g_settings_reset(self->settings_right, (bandn + "-q"s).c_str());
                         }
                       }),
                       self);
    } else {
      // unified mode

      // The left channel reset has to be applied to both channels when not in split mode

      g_signal_connect(reset_frequency, "clicked", G_CALLBACK(+[](GtkButton* btn, EqualizerBox* self) {
                         auto bandn = static_cast<const char*>(g_object_get_data(G_OBJECT(btn), "bandn"));

                         g_settings_reset(self->settings_left, (bandn + "-frequency"s).c_str());
                         g_settings_reset(self->settings_right, (bandn + "-frequency"s).c_str());
                       }),
                       self);

      g_signal_connect(reset_quality, "clicked", G_CALLBACK(+[](GtkButton* btn, EqualizerBox* self) {
                         auto bandn = static_cast<const char*>(g_object_get_data(G_OBJECT(btn), "bandn"));

                         g_settings_reset(self->settings_left, (bandn + "-q"s).c_str());
                         g_settings_reset(self->settings_right, (bandn + "-q"s).c_str());
                       }),
                       self);
    }

    g_signal_connect(band_type, "changed", G_CALLBACK(+[](GtkComboBox* btn, EqualizerBox* self) {
                       // disable gain scale if type is "Off", "Hi-pass" or "Lo-pass"

                       const auto row = gtk_combo_box_get_active(btn);

                       auto* band_scale = GTK_WIDGET(g_object_get_data(G_OBJECT(btn), "band-scale"));

                       gtk_widget_set_sensitive(
                           band_scale, static_cast<gboolean>((row == 0 || row == 2 || row == 4) ? false : true));
                     }),
                     self);

    GSettings* settings;
    GtkBox* bands_box;

    if constexpr (channel == Channel::left) {
      settings = self->settings_left;

      bands_box = self->bands_box_left;
    } else if constexpr (channel == Channel::right) {
      settings = self->settings_right;

      bands_box = self->bands_box_right;
    }

    g_settings_bind(settings, (bandn + "-gain"s).c_str(), gtk_range_get_adjustment(GTK_RANGE(band_scale)), "value",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, (bandn + "-frequency"s).c_str(), gtk_spin_button_get_adjustment(band_frequency), "value",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, (bandn + "-q"s).c_str(), gtk_spin_button_get_adjustment(band_quality), "value",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, (bandn + "-solo"s).c_str(), band_solo, "active", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, (bandn + "-mute"s).c_str(), band_mute, "active", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(
        settings, (bandn + "-type"s).c_str(), band_type, "active", G_SETTINGS_BIND_DEFAULT,
        +[](GValue* value, GVariant* variant, gpointer user_data) {
          const auto* v = g_variant_get_string(variant, nullptr);

          if (g_strcmp0(v, "Off") == 0) {
            g_value_set_int(value, 0);
          } else if (g_strcmp0(v, "Bell") == 0) {
            g_value_set_int(value, 1);
          } else if (g_strcmp0(v, "Hi-pass") == 0) {
            g_value_set_int(value, 2);
          } else if (g_strcmp0(v, "Hi-shelf") == 0) {
            g_value_set_int(value, 3);
          } else if (g_strcmp0(v, "Lo-pass") == 0) {
            g_value_set_int(value, 4);
          } else if (g_strcmp0(v, "Lo-shelf") == 0) {
            g_value_set_int(value, 5);
          } else if (g_strcmp0(v, "Notch") == 0) {
            g_value_set_int(value, 6);
          } else if (g_strcmp0(v, "Resonance") == 0) {
            g_value_set_int(value, 7);
          } else if (g_strcmp0(v, "Allpass") == 0) {
            g_value_set_int(value, 8);
          }

          return 1;
        },
        +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
          switch (g_value_get_int(value)) {
            case 0:
              return g_variant_new_string("Off");
            case 1:
              return g_variant_new_string("Bell");
            case 2:
              return g_variant_new_string("Hi-pass");
            case 3:
              return g_variant_new_string("Hi-shelf");
            case 4:
              return g_variant_new_string("Lo-pass");
            case 5:
              return g_variant_new_string("Lo-shelf");
            case 6:
              return g_variant_new_string("Notch");
            case 7:
              return g_variant_new_string("Resonance");
            case 8:
              return g_variant_new_string("Allpass");
            default:
              return g_variant_new_string("Bell");
          }
        },
        nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, (bandn + "-mode"s).c_str(), band_mode, "active", G_SETTINGS_BIND_DEFAULT,
        +[](GValue* value, GVariant* variant, gpointer user_data) {
          const auto* v = g_variant_get_string(variant, nullptr);

          if (g_strcmp0(v, "RLC (BT)") == 0) {
            g_value_set_int(value, 0);
          } else if (g_strcmp0(v, "RLC (MT)") == 0) {
            g_value_set_int(value, 1);
          } else if (g_strcmp0(v, "BWC (BT)") == 0) {
            g_value_set_int(value, 2);
          } else if (g_strcmp0(v, "BWC (MT)") == 0) {
            g_value_set_int(value, 3);
          } else if (g_strcmp0(v, "LRX (BT)") == 0) {
            g_value_set_int(value, 4);
          } else if (g_strcmp0(v, "LRX (MT)") == 0) {
            g_value_set_int(value, 5);
          } else if (g_strcmp0(v, "APO (DR)") == 0) {
            g_value_set_int(value, 6);
          }

          return 1;
        },
        +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
          switch (g_value_get_int(value)) {
            case 0:
              return g_variant_new_string("RLC (BT)");
            case 1:
              return g_variant_new_string("RLC (MT)");
            case 2:
              return g_variant_new_string("BWC (BT)");
            case 3:
              return g_variant_new_string("BWC (MT)");
            case 4:
              return g_variant_new_string("LRX (BT)");
            case 5:
              return g_variant_new_string("LRX (MT)");
            case 6:
              return g_variant_new_string("APO (DR)");
            default:
              return g_variant_new_string("RLC (BT)");
          }
        },
        nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, (bandn + "-slope"s).c_str(), band_slope, "active", G_SETTINGS_BIND_DEFAULT,
        +[](GValue* value, GVariant* variant, gpointer user_data) {
          const auto* v = g_variant_get_string(variant, nullptr);

          if (g_strcmp0(v, "x1") == 0) {
            g_value_set_int(value, 0);
          } else if (g_strcmp0(v, "x2") == 0) {
            g_value_set_int(value, 1);
          } else if (g_strcmp0(v, "x3") == 0) {
            g_value_set_int(value, 2);
          } else if (g_strcmp0(v, "x4") == 0) {
            g_value_set_int(value, 3);
          }

          return 1;
        },
        +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
          switch (g_value_get_int(value)) {
            case 0:
              return g_variant_new_string("x1");
            case 1:
              return g_variant_new_string("x2");
            case 2:
              return g_variant_new_string("x3");
            case 3:
              return g_variant_new_string("x4");
            default:
              return g_variant_new_string("x1");
          }
        },
        nullptr, nullptr);

    gtk_box_append(bands_box, GTK_WIDGET(band_box));
  }
}

void build_all_bands(EqualizerBox* self) {
  for (auto& handler_id : self->gconnections_left) {
    g_signal_handler_disconnect(self->settings_left, handler_id);
  }

  for (auto& handler_id : self->gconnections_right) {
    g_signal_handler_disconnect(self->settings_right, handler_id);
  }

  self->gconnections_left.clear();
  self->gconnections_right.clear();

  for (auto* child = gtk_widget_get_first_child(GTK_WIDGET(self->bands_box_left)); child != nullptr;) {
    auto* next_child = gtk_widget_get_next_sibling(child);

    gtk_box_remove(self->bands_box_left, child);

    child = next_child;
  }

  for (auto* child = gtk_widget_get_first_child(GTK_WIDGET(self->bands_box_right)); child != nullptr;) {
    auto* next_child = gtk_widget_get_next_sibling(child);

    gtk_box_remove(self->bands_box_right, child);

    child = next_child;
  }

  const auto split = g_settings_get_boolean(self->settings, "split-channels") != 0;

  const auto nbands = g_settings_get_int(self->settings, "num-bands");

  build_channel_bands<Channel::left>(self, nbands, split);

  if (split) {
    build_channel_bands<Channel::right>(self, nbands, split);
  }
}

void setup(EqualizerBox* self,
           std::shared_ptr<Equalizer> equalizer,
           const std::string& schema_path,
           app::Application* application) {
  self->equalizer = equalizer;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer", schema_path.c_str());

  self->settings_left =
      g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel", (schema_path + "leftchannel/").c_str());

  self->settings_right = g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel",
                                                  (schema_path + "rightchannel/").c_str());

  equalizer->post_messages = true;
  equalizer->bypass = false;

  build_all_bands(self);

  self->connections.push_back(equalizer->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->connections.push_back(equalizer->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  g_settings_bind(self->settings, "input-gain", gtk_range_get_adjustment(GTK_RANGE(self->input_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "output-gain", gtk_range_get_adjustment(GTK_RANGE(self->output_gain)), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "num-bands", gtk_spin_button_get_adjustment(self->nbands), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "split-channels", self->split_channels, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(
      self->settings, "mode", self->mode, "active", G_SETTINGS_BIND_DEFAULT,
      +[](GValue* value, GVariant* variant, gpointer user_data) {
        const auto* v = g_variant_get_string(variant, nullptr);

        if (g_strcmp0(v, "IIR") == 0) {
          g_value_set_int(value, 0);
        } else if (g_strcmp0(v, "FIR") == 0) {
          g_value_set_int(value, 1);
        } else if (g_strcmp0(v, "FFT") == 0) {
          g_value_set_int(value, 2);
        } else if (g_strcmp0(v, "SPM") == 0) {
          g_value_set_int(value, 3);
        }

        return 1;
      },
      +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
        switch (g_value_get_int(value)) {
          case 0:
            return g_variant_new_string("IIR");
          case 1:
            return g_variant_new_string("FIR");
          case 2:
            return g_variant_new_string("FFT");
          case 3:
            return g_variant_new_string("SPM");
          default:
            return g_variant_new_string("IIR");
        }
      },
      nullptr, nullptr);

  self->gconnections.push_back(g_signal_connect(
      self->settings, "changed::num-bands",
      G_CALLBACK(+[](GSettings* settings, char* key, EqualizerBox* self) { build_all_bands(self); }), self));

  self->gconnections.push_back(g_signal_connect(self->settings, "changed::split-channels",
                                                G_CALLBACK(+[](GSettings* settings, char* key, EqualizerBox* self) {
                                                  gtk_stack_set_visible_child_name(self->stack, "page_left_channel");

                                                  build_all_bands(self);
                                                }),
                                                self));
}

void dispose(GObject* object) {
  auto* self = EE_EQUALIZER_BOX(object);

  self->equalizer->post_messages = false;
  self->equalizer->bypass = false;

  for (auto& c : self->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  for (auto& handler_id : self->gconnections_left) {
    g_signal_handler_disconnect(self->settings_left, handler_id);
  }

  for (auto& handler_id : self->gconnections_right) {
    g_signal_handler_disconnect(self->settings_right, handler_id);
  }

  self->connections.clear();
  self->gconnections.clear();
  self->gconnections_left.clear();
  self->gconnections_right.clear();

  g_object_unref(self->settings);
  g_object_unref(self->settings_left);
  g_object_unref(self->settings_right);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(equalizer_box_parent_class)->dispose(object);
}

void equalizer_box_class_init(EqualizerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/equalizer.ui");

  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, stack);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, bands_box_left);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, bands_box_right);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, nbands);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, mode);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, split_channels);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void equalizer_box_init(EqualizerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

auto create() -> EqualizerBox* {
  return static_cast<EqualizerBox*>(g_object_new(EE_TYPE_EQUALIZER_BOX, nullptr));
}

}  // namespace ui::equalizer_box

EqualizerUi::EqualizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path,
                         const std::string& schema_channel,
                         const std::string& schema_channel_left_path,
                         const std::string& schema_channel_right_path)
    : Gtk::Box(cobject),
      PluginUiBase(builder, schema, schema_path),
      settings_left(Gio::Settings::create(schema_channel, schema_channel_left_path)),
      settings_right(Gio::Settings::create(schema_channel, schema_channel_right_path)) {
  name = plugin_name::equalizer;

  stack = builder->get_widget<Gtk::Stack>("stack");
  stack_switcher = builder->get_widget<Gtk::StackSwitcher>("stack_switcher");

  // signals connections

  // reset equalizer
  flat_response->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_flat_response));

  calculate_freqs->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_calculate_frequencies));

  import_apo->signal_clicked().connect(sigc::mem_fun(*this, &EqualizerUi::on_import_apo_preset_clicked));
}

EqualizerUi::~EqualizerUi() {
  for (auto& c : connections_bands) {
    c.disconnect();
  }

  util::debug(name + " ui destroyed");
}

void EqualizerUi::on_flat_response() {
  for (int n = 0; n < max_bands; n++) {
    const auto bandn = "band" + std::to_string(n);

    // left channel

    settings_left->reset(bandn + "-gain");

    // right channel

    settings_right->reset(bandn + "-gain");
  }
}

void EqualizerUi::on_calculate_frequencies() {
  static const double min_freq = 20.0;
  static const double max_freq = 20000.0;

  double freq0 = min_freq;
  double freq1 = 0.0;

  const auto nbands = settings->get_int("num-bands");

  // code taken from gstreamer equalizer sources: gstiirequalizer.c
  // function: gst_iir_equalizer_compute_frequencies

  const double step = std::pow(max_freq / min_freq, 1.0 / static_cast<double>(nbands));

  auto config_band = [&](const auto& cfg, const auto& n, const auto& freq, const auto& q) {
    const auto bandn = "band" + std::to_string(n);

    cfg->set_double(bandn + "-frequency", freq);

    cfg->set_double(bandn + "-q", q);
  };

  for (int n = 0; n < nbands; n++) {
    freq1 = freq0 * step;

    const double freq = freq0 + 0.5 * (freq1 - freq0);
    const double width = freq1 - freq0;
    const double q = freq / width;

    // std::cout << n << "\t" << freq << "\t" << width << std::endl;

    config_band(settings_left, n, freq, q);
    config_band(settings_right, n, freq, q);

    freq0 = freq1;
  }
}

void EqualizerUi::on_import_apo_preset_clicked() {
  Glib::RefPtr<Gtk::FileChooserNative> dialog;

  if (transient_window != nullptr) {
    dialog = Gtk::FileChooserNative::create(_("Import APO Preset File"), *transient_window,
                                            Gtk::FileChooser::Action::OPEN, _("Open"), _("Cancel"));
  } else {
    dialog = Gtk::FileChooserNative::create(_("Import APO Preset File"), Gtk::FileChooser::Action::OPEN, _("Open"),
                                            _("Cancel"));
  }

  auto dialog_filter = Gtk::FileFilter::create();

  dialog_filter->set_name(_("APO Presets"));
  dialog_filter->add_pattern("*.txt");

  dialog->add_filter(dialog_filter);

  dialog->signal_response().connect([=, this](const auto& response_id) {
    switch (response_id) {
      case Gtk::ResponseType::ACCEPT: {
        import_apo_preset(dialog->get_file()->get_path());

        break;
      }
      default:
        break;
    }
  });

  dialog->set_modal(true);
  dialog->show();
}

// returns false if we cannot parse given line successfully
bool EqualizerUi::parse_apo_preamp(const std::string& line, double& preamp) {
  std::smatch matches;

  static const auto i = std::regex::icase;

  std::regex_search(line, matches, std::regex(R"(preamp:\s*+([+-]?+\d++(?:\.\d++)*+)\s*+db)", i));

  if (matches.size() != 2U) {
    return false;
  }

  preamp = std::stod(matches.str(1));

  return true;
}

// returns false if we cannot parse given line successfully

auto EqualizerUi::parse_apo_filter(const std::string& line, struct ImportedBand& filter) -> bool {
  std::smatch matches;

  static const auto i = std::regex::icase;

  // get filter type

  std::regex_search(line, matches, std::regex(R"(filter\s++\d*+:\s*+on\s++([a-z]++))", i));

  if (matches.size() != 2U) {
    return false;
  }

  try {
    filter.type = EqualizerUi::FilterTypeMap.at(matches.str(1));
  } catch (...) {
    return false;
  }

  // get center frequency

  std::regex_search(line, matches, std::regex(R"(fc\s++(\d++(?:,\d++)*+(?:\.\d++)*+)\s*+hz)", i));

  if (matches.size() != 2U) {
    return false;
  }

  // frequency could have a comma as thousands separator to be removed

  filter.freq = std::stof(std::regex_replace(matches.str(1), std::regex(","), ""));

  // get slope

  if ((filter.type & (LOW_SHELF_xdB | HIGH_SHELF_xdB | LOW_SHELF | HIGH_SHELF)) != 0U) {
    std::regex_search(line, matches,
                      std::regex(R"(filter\s++\d*+:\s*+on\s++[a-z]++\s++([+-]?+\d++(?:\.\d++)*+)\s*+db)", i));

    // _xdB variants require the dB parameter

    if (((filter.type & (LOW_SHELF_xdB | HIGH_SHELF_xdB)) != 0U) && (matches.size() != 2U)) {
      return false;
    }

    if (matches.size() == 2U) {
      // we satisfied the condition, now assign the paramater if given

      filter.slope_dB = std::stof(matches.str(1));
    }
  }

  // get gain

  if ((filter.type & (PEAKING | LOW_SHELF_xdB | HIGH_SHELF_xdB | LOW_SHELF | HIGH_SHELF)) != 0U) {
    std::regex_search(line, matches, std::regex(R"(gain\s++([+-]?+\d++(?:\.\d++)*+)\s*+db)", i));

    // all Shelf types (i.e. all above except for Peaking) require the gain parameter

    if (((filter.type & PEAKING) == 0U) && (matches.size() != 2U)) {
      return false;
    }

    if (matches.size() == 2U) {
      filter.gain = std::stof(matches.str(1));
    }
  }

  // get quality factor
  if ((filter.type & (PEAKING | LOW_PASS_Q | HIGH_PASS_Q | LOW_SHELF_xdB | HIGH_SHELF_xdB | NOTCH | ALL_PASS)) != 0U) {
    std::regex_search(line, matches, std::regex(R"(q\s++(\d++(?:\.\d++)*+))", i));

    // Peaking and All-Pass filter types require the quality factor parameter

    if (((filter.type & (PEAKING | ALL_PASS)) != 0U) && (matches.size() != 2U)) {
      return false;
    }

    if (matches.size() == 2U) {
      filter.quality_factor = std::stof(matches.str(1));
    }
  }

  return true;
}

void EqualizerUi::import_apo_preset(const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (std::filesystem::is_regular_file(p)) {
    std::ifstream eq_file;
    std::vector<struct ImportedBand> bands;
    double preamp = 0.0;

    eq_file.open(p.c_str());

    if (eq_file.is_open()) {
      std::string line;

      while (getline(eq_file, line)) {
        struct ImportedBand filter {};

        if (!parse_apo_preamp(line, preamp)) {
          if (parse_apo_filter(line, filter)) {
            bands.push_back(filter);
          }
        }
      }
    }

    eq_file.close();

    if (bands.empty()) {
      return;
    }

    settings->set_int("num-bands", bands.size());
    settings->set_double("input-gain", preamp);

    for (int n = 0; n < max_bands; n++) {
      const auto bandn = "band" + std::to_string(n);

      if (n < static_cast<int>(bands.size())) {
        settings_left->set_string(bandn + "-mode", "APO (DR)");

        if (!settings->get_boolean("split-channels")) {
          settings_left->set_string(bandn + "-type", "Bell");
          settings_left->set_double(bandn + "-gain", bands[n].gain);
          settings_left->set_double(bandn + "-frequency", bands[n].freq);
          settings_left->set_double(bandn + "-q", bands[n].quality_factor);

          settings_right->set_string(bandn + "-type", "Bell");
          settings_right->set_double(bandn + "-gain", bands[n].gain);
          settings_right->set_double(bandn + "-frequency", bands[n].freq);
          settings_right->set_double(bandn + "-q", bands[n].quality_factor);
        } else {
          if (stack->get_visible_child_name() == "page_left_channel") {
            settings_left->set_string(bandn + "-type", "Bell");
            settings_left->set_double(bandn + "-gain", bands[n].gain);
            settings_left->set_double(bandn + "-frequency", bands[n].freq);
            settings_left->set_double(bandn + "-q", bands[n].quality_factor);
          } else {
            settings_right->set_string(bandn + "-type", "Bell");
            settings_right->set_double(bandn + "-gain", bands[n].gain);
            settings_right->set_double(bandn + "-frequency", bands[n].freq);
            settings_right->set_double(bandn + "-q", bands[n].quality_factor);
          }
        }
      } else {
        settings_left->set_string(bandn + "-type", "Off");

        settings_right->set_string(bandn + "-type", "Off");
      }
    }
  }
}
