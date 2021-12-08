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
using namespace tags::equalizer;

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

  app::Application* application;

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

void on_flat_response(EqualizerBox* self, GtkButton* btn) {
  for (int n = 0; n < max_bands; n++) {
    g_settings_reset(self->settings_left, band_gain[n]);

    g_settings_reset(self->settings_right, band_gain[n]);
  }
}

void on_calculate_frequencies(EqualizerBox* self, GtkButton* btn) {
  static const double min_freq = 20.0;
  static const double max_freq = 20000.0;

  double freq0 = min_freq;
  double freq1 = 0.0;

  const auto nbands = g_settings_get_int(self->settings, "num-bands");

  // code taken from gstreamer equalizer sources: gstiirequalizer.c
  // function: gst_iir_equalizer_compute_frequencies

  const double step = std::pow(max_freq / min_freq, 1.0 / static_cast<double>(nbands));

  for (int n = 0; n < nbands; n++) {
    freq1 = freq0 * step;

    const double freq = freq0 + 0.5 * (freq1 - freq0);
    const double width = freq1 - freq0;
    const double q = freq / width;

    // std::cout << n << "\t" << freq << "\t" << width << std::endl;

    g_settings_set_double(self->settings_left, band_frequency[n], freq);
    g_settings_set_double(self->settings_left, band_q[n], q);

    g_settings_set_double(self->settings_right, band_frequency[n], freq);
    g_settings_set_double(self->settings_right, band_q[n], q);

    freq0 = freq1;
  }
}

// returns false if we cannot parse given line successfully
auto parse_apo_preamp(const std::string& line, double& preamp) -> bool {
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
auto parse_apo_filter(const std::string& line, struct ImportedBand& filter) -> bool {
  std::smatch matches;

  static const auto i = std::regex::icase;

  // get filter type

  std::regex_search(line, matches, std::regex(R"(filter\s++\d*+:\s*+on\s++([a-z]++))", i));

  if (matches.size() != 2U) {
    return false;
  }

  try {
    filter.type = FilterTypeMap.at(matches.str(1));
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

void import_apo_preset(EqualizerBox* self, const std::string& file_path) {
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

    g_settings_set_int(self->settings, "num-bands", bands.size());
    g_settings_set_double(self->settings, "input-gain", preamp);

    for (int n = 0; n < max_bands; n++) {
      if (n < static_cast<int>(bands.size())) {
        g_settings_set_string(self->settings, band_mode[n], "APO (DR)");

        if (g_settings_get_boolean(self->settings, "split-channels") == 0) {
          g_settings_set_string(self->settings_left, band_type[n], "Bell");
          g_settings_set_double(self->settings_left, band_gain[n], bands[n].gain);
          g_settings_set_double(self->settings_left, band_frequency[n], bands[n].freq);
          g_settings_set_double(self->settings_left, band_q[n], bands[n].quality_factor);

          g_settings_set_string(self->settings_right, band_type[n], "Bell");
          g_settings_set_double(self->settings_right, band_gain[n], bands[n].gain);
          g_settings_set_double(self->settings_right, band_frequency[n], bands[n].freq);
          g_settings_set_double(self->settings_right, band_q[n], bands[n].quality_factor);
        } else {
          /*
            The code below allows users to load different APO presets for each channel
          */

          if (g_strcmp0(gtk_stack_get_visible_child_name(self->stack), "page_left_channel") == 0) {
            g_settings_set_string(self->settings_left, band_type[n], "Bell");
            g_settings_set_double(self->settings_left, band_gain[n], bands[n].gain);
            g_settings_set_double(self->settings_left, band_frequency[n], bands[n].freq);
            g_settings_set_double(self->settings_left, band_q[n], bands[n].quality_factor);
          } else {
            g_settings_set_string(self->settings_right, band_type[n], "Bell");
            g_settings_set_double(self->settings_right, band_gain[n], bands[n].gain);
            g_settings_set_double(self->settings_right, band_frequency[n], bands[n].freq);
            g_settings_set_double(self->settings_right, band_q[n], bands[n].quality_factor);
          }
        }
      } else {
        g_settings_set_string(self->settings_left, band_type[n], "Off");
        g_settings_set_string(self->settings_right, band_type[n], "Off");
      }
    }
  }
}

void on_import_apo_preset_clicked(EqualizerBox* self, GtkButton* btn) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->application));

  auto* dialog = gtk_file_chooser_native_new(_("Import APO Preset File"), active_window, GTK_FILE_CHOOSER_ACTION_OPEN,
                                             _("Open"), _("Cancel"));

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_add_pattern(filter, "*.txt");
  gtk_file_filter_set_name(filter, _("APO Presets"));

  gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

  g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkNativeDialog* dialog, int response, EqualizerBox* self) {
                     if (response == GTK_RESPONSE_ACCEPT) {
                       auto* chooser = GTK_FILE_CHOOSER(dialog);
                       auto* file = gtk_file_chooser_get_file(chooser);
                       auto* path = g_file_get_path(file);

                       import_apo_preset(self, path);

                       g_object_unref(file);
                     }

                     g_object_unref(dialog);
                   }),
                   self);

  gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(dialog), 1);
  gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

template <Channel channel>
void build_channel_bands(EqualizerBox* self, const int& nbands, const bool& split_mode) {
  GSettings* settings;
  GtkBox* bands_box;

  if constexpr (channel == Channel::left) {
    settings = self->settings_left;

    bands_box = self->bands_box_left;
  } else if constexpr (channel == Channel::right) {
    settings = self->settings_right;

    bands_box = self->bands_box_right;
  }

  for (int n = 0; n < nbands; n++) {
    auto band_box = ui::equalizer_band_box::create();

    ui::equalizer_band_box::setup(band_box, settings, n);

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

  self->application = application;

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

  g_settings_bind(self->settings, "mode", self->mode, "active-id", G_SETTINGS_BIND_DEFAULT);

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

  gtk_widget_class_bind_template_callback(widget_class, on_flat_response);
  gtk_widget_class_bind_template_callback(widget_class, on_calculate_frequencies);
  gtk_widget_class_bind_template_callback(widget_class, on_import_apo_preset_clicked);
}

void equalizer_box_init(EqualizerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

auto create() -> EqualizerBox* {
  return static_cast<EqualizerBox*>(g_object_new(EE_TYPE_EQUALIZER_BOX, nullptr));
}

}  // namespace ui::equalizer_box
