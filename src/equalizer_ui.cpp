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

struct APO_Band {
  std::string type;
  float freq = 1000.0f;
  float gain = 0.0f;
  float quality = (1.0f / std::numbers::sqrt2_v<float>);
};

std::unordered_map<std::string, std::string> const FilterTypeMap = {
    {"PK", "Bell"},          {"MODAL", "Bell"},  {"PEQ", "Bell"},     {"LP", "Lo-pass"},      {"LPQ", "Lo-pass"},
    {"HP", "Hi-pass"},       {"HPQ", "Hi-pass"}, {"LS", "Lo-shelf"},  {"LSC", "Lo-shelf"},    {"LS 6DB", "Lo-shelf"},
    {"LS 12DB", "Lo-shelf"}, {"HS", "Hi-shelf"}, {"HSC", "Hi-shelf"}, {"HS 6DB", "Hi-shelf"}, {"HS 12DB", "Hi-shelf"},
    {"NO", "Notch"},         {"AP", "Allpass"}};

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  app::Application* application;

  std::shared_ptr<Equalizer> equalizer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections, gconnections_left, gconnections_right;
};

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

  Data* data;
};

G_DEFINE_TYPE(EqualizerBox, equalizer_box, GTK_TYPE_BOX)

void on_bypass(EqualizerBox* self, GtkToggleButton* btn) {
  self->data->equalizer->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(EqualizerBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
  util::reset_all_keys(self->settings_left);
  util::reset_all_keys(self->settings_right);
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

auto parse_apo_preamp(const std::string& line, double& preamp) -> bool {
  std::smatch matches;

  static const auto re_preamp = std::regex(R"(preamp\s*+:\s*+([+-]?+\d++(?:\.\d++)?+)\s*+db)", std::regex::icase);

  std::regex_search(line, matches, re_preamp);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), preamp);
}

auto parse_apo_filter(const std::string& line, struct APO_Band& filter) -> std::string {
  std::smatch matches;

  static const auto re_filter =
      std::regex(R"(filter\s*+\d*+\s*+:\s*+on\s++([a-z]++(?:\s++(?:6|12)db)?+))", std::regex::icase);

  std::regex_search(line, matches, re_filter);

  if (matches.size() != 2U) {
    return "";
  }

  // Possible multiple whitespaces are replaced by a single space
  auto apo_filter = std::regex_replace(matches.str(1), std::regex(R"(\s++)"), " ");

  // Filter string needed in uppercase for unordered_map
  std::transform(apo_filter.begin(), apo_filter.end(), apo_filter.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  try {
    filter.type = FilterTypeMap.at(apo_filter);
  } catch (...) {
    filter.type = "Off";
  }

  return apo_filter;
}

auto parse_apo_frequency(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_freq = std::regex(R"(fc\s++(\d++(?:,\d++)?+(?:\.\d++)?+)\s*+hz)", std::regex::icase);

  std::regex_search(line, matches, re_freq);

  if (matches.size() != 2U) {
    return false;
  }

  // Frequency could have a comma as thousands separator
  // to be removed for the correct float conversion.
  return util::str_to_num(std::regex_replace(matches.str(1), std::regex(","), ""), filter.freq);
}

auto parse_apo_gain(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_gain = std::regex(R"(gain\s++([+-]?+\d++(?:\.\d++)?+)\s*+db)", std::regex::icase);

  std::regex_search(line, matches, re_gain);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), filter.gain);
}

auto parse_apo_quality(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_quality = std::regex(R"(q\s++(\d++(?:\.\d++)?+))", std::regex::icase);

  std::regex_search(line, matches, re_quality);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), filter.quality);
}

auto parse_apo_config_line(const std::string& line, struct APO_Band& filter) -> bool {
  std::string filter_type = parse_apo_filter(line, filter);

  if (filter_type.empty()) {
    return false;
  }

  // The configuration line refers to an existing APO filter, so we try to get the other parameters.
  parse_apo_frequency(line, filter);

  // Inspired by funtion "para_equalizer_ui::import_rew_file(const LSPString*)"
  // inside 'lsp-plugins/src/ui/plugins/para_equalizer_ui.cpp' at
  // https://github.com/sadko4u/lsp-plugins
  if (filter_type == "PK" || filter_type == "MODAL" || filter_type == "PEQ") {
    parse_apo_gain(line, filter);

    parse_apo_quality(line, filter);
  } else if (filter_type == "LP" || filter_type == "LPQ" || filter_type == "HP" || filter_type == "HPQ") {
    parse_apo_quality(line, filter);
  } else if (filter_type == "LS" || filter_type == "LSC" || filter_type == "HS" || filter_type == "HSC") {
    parse_apo_gain(line, filter);

    if (!parse_apo_quality(line, filter)) {
      filter.quality = 2.0f / 3.0f;
    }
  } else if (filter_type == "LS 6DB") {
    filter.freq = filter.freq * 2.0f / 3.0f;
    filter.quality = std::numbers::sqrt2_v<float> / 3.0f;

    parse_apo_gain(line, filter);
  } else if (filter_type == "LS 12DB") {
    filter.freq = filter.freq * 3.0f / 2.0f;

    parse_apo_gain(line, filter);
  } else if (filter_type == "HS 6DB") {
    filter.freq = filter.freq / (1.0f / std::numbers::sqrt2_v<float>);
    filter.quality = std::numbers::sqrt2_v<float> / 3.0f;

    parse_apo_gain(line, filter);
  } else if (filter_type == "HS 12DB") {
    filter.freq = filter.freq * (1.0f / std::numbers::sqrt2_v<float>);

    parse_apo_gain(line, filter);
  } else if (filter_type == "NO") {
    if (!parse_apo_quality(line, filter)) {
      filter.quality = 100.0f / 3.0f;
    }
  } else if (filter_type == "AP") {
    parse_apo_quality(line, filter);
  }

  // If the APO filter type is different than the ones specified above,
  // it's set as Off since it's not supported by LSP Equalizer.
  // Default values are assumed.
  return true;
}

void import_apo_preset(EqualizerBox* self, const std::string& file_path) {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    return;
  }

  std::ifstream eq_file;
  eq_file.open(p.c_str());

  std::vector<struct APO_Band> bands;
  double preamp = 0.0;

  if (eq_file.is_open()) {
    for (std::string line; getline(eq_file, line);) {
      if (struct APO_Band filter; parse_apo_config_line(line, filter)) {
        bands.push_back(filter);
      } else {
        parse_apo_preamp(line, preamp);
      }
    }
  }

  eq_file.close();

  if (bands.empty()) {
    return;
  }

  // Apply APO parameters obtained
  g_settings_set_int(self->settings, "num-bands", static_cast<int>(bands.size()));
  g_settings_set_double(self->settings, "input-gain", preamp);

  std::vector<GSettings*> settings_channels;

  // Whether to apply the parameters to both channels or the selected one only
  if (g_settings_get_boolean(self->settings, "split-channels") == 0) {
    settings_channels.push_back(self->settings_left);
    settings_channels.push_back(self->settings_right);
  } else if (g_strcmp0(gtk_stack_get_visible_child_name(self->stack), "page_left_channel") == 0) {
    settings_channels.push_back(self->settings_left);
  } else {
    settings_channels.push_back(self->settings_right);
  }

  for (int n = 0, apo_bands = static_cast<int>(bands.size()); n < max_bands; n++) {
    for (auto* channel : settings_channels) {
      if (n < apo_bands) {
        g_settings_set_string(channel, band_type[n], bands[n].type.c_str());
        g_settings_set_string(channel, band_mode[n], "APO (DR)");
        g_settings_set_double(channel, band_frequency[n], bands[n].freq);
        g_settings_set_double(channel, band_gain[n], bands[n].gain);
        g_settings_set_double(channel, band_q[n], bands[n].quality);
      } else {
        g_settings_set_string(channel, band_type[n], "Off");
        g_settings_reset(channel, band_mode[n]);
        g_settings_reset(channel, band_frequency[n]);
        g_settings_reset(channel, band_gain[n]);
        g_settings_reset(channel, band_q[n]);
      }

      g_settings_reset(channel, band_slope[n]);
      g_settings_reset(channel, band_solo[n]);
      g_settings_reset(channel, band_mute[n]);
    }
  }
}

void on_import_apo_preset_clicked(EqualizerBox* self, GtkButton* btn) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

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

                       g_free(path);

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
  for (auto& handler_id : self->data->gconnections_left) {
    g_signal_handler_disconnect(self->settings_left, handler_id);
  }

  for (auto& handler_id : self->data->gconnections_right) {
    g_signal_handler_disconnect(self->settings_right, handler_id);
  }

  self->data->gconnections_left.clear();
  self->data->gconnections_right.clear();

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
  self->data->equalizer = equalizer;

  self->data->application = application;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer", schema_path.c_str());

  self->settings_left =
      g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel", (schema_path + "leftchannel/").c_str());

  self->settings_right = g_settings_new_with_path("com.github.wwmm.easyeffects.equalizer.channel",
                                                  (schema_path + "rightchannel/").c_str());

  equalizer->post_messages = true;
  equalizer->bypass = false;

  build_all_bands(self);

  self->data->connections.push_back(equalizer->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(equalizer->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "num-bands", gtk_spin_button_get_adjustment(self->nbands), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "split-channels", self->split_channels, "active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "mode", self->mode, "active-id", G_SETTINGS_BIND_DEFAULT);

  self->data->gconnections.push_back(g_signal_connect(
      self->settings, "changed::num-bands",
      G_CALLBACK(+[](GSettings* settings, char* key, EqualizerBox* self) { build_all_bands(self); }), self));

  self->data->gconnections.push_back(g_signal_connect(
      self->settings, "changed::split-channels", G_CALLBACK(+[](GSettings* settings, char* key, EqualizerBox* self) {
        gtk_stack_set_visible_child_name(self->stack, "page_left_channel");

        build_all_bands(self);
      }),
      self));
}

void dispose(GObject* object) {
  auto* self = EE_EQUALIZER_BOX(object);

  self->data->equalizer->bypass = false;

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  for (auto& handler_id : self->data->gconnections_left) {
    g_signal_handler_disconnect(self->settings_left, handler_id);
  }

  for (auto& handler_id : self->data->gconnections_right) {
    g_signal_handler_disconnect(self->settings_right, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();
  self->data->gconnections_left.clear();
  self->data->gconnections_right.clear();

  g_object_unref(self->settings);
  g_object_unref(self->settings_left);
  g_object_unref(self->settings_right);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(equalizer_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_EQUALIZER_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(equalizer_box_parent_class)->finalize(object);
}

void equalizer_box_class_init(EqualizerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

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

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);
}

auto create() -> EqualizerBox* {
  return static_cast<EqualizerBox*>(g_object_new(EE_TYPE_EQUALIZER_BOX, nullptr));
}

}  // namespace ui::equalizer_box
