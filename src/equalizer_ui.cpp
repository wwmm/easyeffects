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

#include "equalizer_ui.hpp"
#include <STTypes.h>
#include <adwaita.h>
#include <gio/gio.h>
#include <gio/gliststore.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glibconfig.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
#include <gtk/gtkdropdown.h>
#include <sigc++/connection.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <numbers>
#include <numeric>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "application.hpp"
#include "equalizer.hpp"
#include "equalizer_band_box.hpp"
#include "tags_equalizer.hpp"
#include "tags_resources.hpp"
#include "tags_schema.hpp"
#include "ui_helpers.hpp"
#include "util.hpp"

namespace ui::equalizer_box {

using namespace tags::equalizer;

enum Channel { left, right };

struct APO_Band {
  std::string type;
  float freq = 1000.0F;
  float gain = 0.0F;
  float quality = (1.0F / std::numbers::sqrt2_v<float>);  // default in LSP APO import
};

struct GraphicEQ_Band {
  float freq = 1000.0F;
  float gain = 0.0F;
};

std::map<std::string, std::string> const ApoToEasyEffectsFilter = {
    {"OFF", "Off"},         {"PK", "Bell"},          {"MODAL", "Bell"},       {"PEQ", "Bell"},    {"LP", "Lo-pass"},
    {"LPQ", "Lo-pass"},     {"HP", "Hi-pass"},       {"HPQ", "Hi-pass"},      {"BP", "Bandpass"}, {"LS", "Lo-shelf"},
    {"LSC", "Lo-shelf"},    {"LS 6DB", "Lo-shelf"},  {"LS 12DB", "Lo-shelf"}, {"HS", "Hi-shelf"}, {"HSC", "Hi-shelf"},
    {"HS 6DB", "Hi-shelf"}, {"HS 12DB", "Hi-shelf"}, {"NO", "Notch"},         {"AP", "Allpass"}};

std::map<std::string, std::string> const EasyEffectsToApoFilter = {
    {"Bell", "PK"},      {"Lo-pass", "LPQ"}, {"Hi-pass", "HPQ"}, {"Lo-shelf", "LSC"},
    {"Hi-shelf", "HSC"}, {"Notch", "NO"},    {"Allpass", "AP"},  {"Bandpass", "BP"}};

struct Data {
 public:
  ~Data() { util::debug("data struct destroyed"); }

  uint serial = 0U;

  app::Application* application{};

  std::shared_ptr<Equalizer> equalizer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _EqualizerBox {
  GtkBox parent_instance;

  AdwToastOverlay* toast_overlay;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label,
      *plugin_credit;

  GtkStack* stack;

  GtkListView *listview_left, *listview_right;

  GtkSpinButton *nbands, *balance, *pitch_left, *pitch_right;

  GtkDropDown* mode;

  GtkToggleButton *split_channels, *show_native_ui;

  GSettings *settings, *settings_left, *settings_right;

  GtkStringList *string_list_left, *string_list_right;

  Data* data;
};

// NOLINTNEXTLINE
G_DEFINE_TYPE(EqualizerBox, equalizer_box, GTK_TYPE_BOX)

void on_reset(EqualizerBox* self, GtkButton* btn) {
  util::reset_all_keys_except(self->settings);
  util::reset_all_keys_except(self->settings_left);
  util::reset_all_keys_except(self->settings_right);
}

void on_show_native_window(EqualizerBox* self, GtkToggleButton* btn) {
  if (gtk_toggle_button_get_active(btn) != 0) {
    self->data->equalizer->show_native_ui();
  } else {
    self->data->equalizer->close_native_ui();
  }
}

void on_flat_response(EqualizerBox* self, GtkButton* btn) {
  const auto& max_bands = self->data->equalizer->max_bands;
  for (uint n = 0U; n < max_bands; n++) {
    g_settings_reset(self->settings_left, band_gain[n].data());

    g_settings_reset(self->settings_right, band_gain[n].data());
  }
}

void on_calculate_frequencies(EqualizerBox* self, GtkButton* btn) {
  constexpr double min_freq = 20.0;
  constexpr double max_freq = 20000.0;

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

    g_settings_set_double(self->settings_left, band_frequency[n].data(), freq);
    g_settings_set_double(self->settings_left, band_q[n].data(), q);
    g_settings_reset(self->settings_left, band_width[n].data());

    g_settings_set_double(self->settings_right, band_frequency[n].data(), freq);
    g_settings_set_double(self->settings_right, band_q[n].data(), q);
    g_settings_reset(self->settings_right, band_width[n].data());

    freq0 = freq1;
  }
}

// ### APO Preset Section ###

auto parse_apo_preamp(const std::string& line, double& preamp) -> bool {
  std::smatch matches;

  static const auto re_preamp = std::regex(R"(preamp\s*:\s*([+-]?\d+(?:\.\d+)?)\s*db)", std::regex::icase);

  std::regex_search(line, matches, re_preamp);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), preamp);
}

auto parse_apo_filter_type(const std::string& line, struct APO_Band& filter) -> bool {
  // Look for disabled filter.
  std::smatch matches_off;

  static const auto re_filter_off = std::regex(R"(filter\s*\d*\s*:\s*off\s)", std::regex::icase);

  std::regex_search(line, matches_off, re_filter_off);

  if (matches_off.size() == 1U) {
    // If the APO filter is disabled, we assume the "OFF" type.
    filter.type = "OFF";

    return true;
  }

  // If the filter is enabled, look for the filter type.
  std::smatch matches_filter;

  static const auto re_filter_type =
      std::regex(R"(filter\s*\d*\s*:\s*on\s+([a-z]+(?:\s+(?:6|12)db)?))", std::regex::icase);

  std::regex_search(line, matches_filter, re_filter_type);

  if (matches_filter.size() != 2U) {
    return false;
  }

  // Possible multiple whitespaces are replaced by a single space
  filter.type = std::regex_replace(matches_filter.str(1), std::regex(R"(\s+)"), " ");

  // Filter string needed in uppercase for lookup in map
  std::transform(filter.type.begin(), filter.type.end(), filter.type.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  return !filter.type.empty();
}

auto parse_apo_frequency(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_freq = std::regex(R"(fc\s+(\d+(?:,\d+)?(?:\.\d+)?)\s*hz)", std::regex::icase);

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

  static const auto re_gain = std::regex(R"(gain\s+([+-]?\d+(?:\.\d+)?)\s*db)", std::regex::icase);

  std::regex_search(line, matches, re_gain);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), filter.gain);
}

auto parse_apo_quality(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_quality = std::regex(R"(q\s+(\d+(?:\.\d+)?))", std::regex::icase);

  std::regex_search(line, matches, re_quality);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), filter.quality);
}

auto parse_apo_config_line(const std::string& line, struct APO_Band& filter) -> bool {
  // Retrieve filter type.
  if (!parse_apo_filter_type(line, filter)) {
    // If we can't parse the filter type, there's something wrong in the text line,
    // so exit with false.
    return false;
  }

  // Retrieve frequency.
  // To make it more permissive, we do not exit on false here (assume default).
  parse_apo_frequency(line, filter);

  // The following has been inspired by the function
  // "para_equalizer_ui::import_rew_file(const LSPString*)"
  // inside 'lsp-plugins/src/ui/plugins/para_equalizer_ui.cpp' at
  // https://github.com/sadko4u/lsp-plugins

  // Retrieve gain and/or quality parameters based on a specific filter type.
  // Calculate frequency/quality if needed.
  // If the APO filter type is different than the ones specified below,
  // it's set as "Off" and default values are assumed since
  // it may not be supported by LSP Equalizer.
  if (filter.type == "OFF") {
    // On disabled filter state, we still try to retrieve gain and quality,
    // even if the band won't be processed by LSP equalizer.
    parse_apo_gain(line, filter);

    parse_apo_quality(line, filter);
  } else if (filter.type == "PK" || filter.type == "MODAL" || filter.type == "PEQ") {
    // Peak/Bell filter
    parse_apo_gain(line, filter);

    parse_apo_quality(line, filter);
  } else if (filter.type == "LP" || filter.type == "LPQ" || filter.type == "HP" || filter.type == "HPQ" ||
             filter.type == "BP") {
    // Low-pass, High-pass and Band-pass filters,
    // (LSP does not import Band-pass, but we do it anyway).
    parse_apo_quality(line, filter);
  } else if (filter.type == "LS" || filter.type == "LSC" || filter.type == "HS" || filter.type == "HSC") {
    // Low-shelf and High-shelf filters (with center freq., x dB per oct.)
    parse_apo_gain(line, filter);

    // Q value is optional for these filters according to APO config documentation,
    // but LSP import function always sets it to 2/3.
    filter.quality = 2.0F / 3.0F;
  } else if (filter.type == "LS 6DB") {
    // Low-shelf filter (6 dB per octave with corner freq.)
    parse_apo_gain(line, filter);

    // LSP import function sets custom freq and quality for this filter.
    filter.freq = filter.freq * 2.0F / 3.0F;
    filter.quality = std::numbers::sqrt2_v<float> / 3.0F;
  } else if (filter.type == "LS 12DB") {
    // Low-shelf filter (12 dB per octave with corner freq.)
    parse_apo_gain(line, filter);

    // LSP import function sets custom freq for this filter.
    filter.freq = filter.freq * 3.0F / 2.0F;
  } else if (filter.type == "HS 6DB") {
    // High-shelf filter (6 dB per octave with corner freq.)
    parse_apo_gain(line, filter);

    // LSP import function sets custom freq and quality for this filter.
    filter.freq = filter.freq / (1.0F / std::numbers::sqrt2_v<float>);
    filter.quality = std::numbers::sqrt2_v<float> / 3.0F;
  } else if (filter.type == "HS 12DB") {
    // High-shelf filter (12 dB per octave with corner freq.)
    parse_apo_gain(line, filter);

    // LSP import function sets custom freq for this filter.
    filter.freq = filter.freq * (1.0F / std::numbers::sqrt2_v<float>);
  } else if (filter.type == "NO") {
    // Notch filter
    // Q value is optional for this filter according to APO config documentation,
    // but LSP import function always sets it to 100/3.
    filter.quality = 100.0F / 3.0F;
  } else if (filter.type == "AP") {
    // All-pass filter
    // Q value is mandatory for this filter according to APO config documentation,
    // but LSP import function always sets it to 0,
    // no matter which quality value the APO config has.
    filter.quality = 0.0F;
  }

  return true;
}

auto import_apo_preset(EqualizerBox* self, const std::string& file_path) -> bool {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    return false;
  }

  std::ifstream eq_file;
  eq_file.open(p.c_str());

  std::vector<struct APO_Band> bands;
  double preamp = 0.0;

  if (const auto re = std::regex(R"(^[ \t]*#)"); eq_file.is_open()) {
    for (std::string line; std::getline(eq_file, line);) {
      if (std::regex_search(line, re)) {  // Avoid commented lines
        continue;
      }

      if (struct APO_Band filter; parse_apo_config_line(line, filter)) {
        bands.push_back(filter);
      } else {
        parse_apo_preamp(line, preamp);
      }
    }
  }

  eq_file.close();

  if (bands.empty()) {
    return false;
  }

  /* Sort bands by freq is made by user through Equalizer::sort_bands()
  std::ranges::stable_sort(bands, {}, &APO_Band::freq); */

  const auto& max_bands = self->data->equalizer->max_bands;

  // Apply APO parameters obtained
  g_settings_set_int(self->settings, "num-bands",
                     static_cast<int>(std::min(static_cast<uint>(bands.size()), max_bands)));
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

  // Retrieve GSettingsSchema* for range check
  GSettingsSchema* schema = nullptr;

  g_object_get(self->settings_left, "settings-schema", &schema, nullptr);

  if (schema == nullptr) {
    return false;
  }

  // Apply APO parameters obtained for each band
  for (uint n = 0U, apo_bands = bands.size(); n < max_bands; n++) {
    for (auto* channel : settings_channels) {
      if (n < apo_bands) {
        // Band frequency and type
        auto* freq_schema_key = g_settings_schema_get_key(schema, band_frequency[n].data());
        auto* freq_variant = g_variant_new_double(static_cast<gdouble>(bands[n].freq));

        if (g_settings_schema_key_range_check(freq_schema_key, freq_variant) != 0) {
          g_settings_set_double(channel, band_frequency[n].data(), bands[n].freq);

          std::string curr_band_type;

          try {
            curr_band_type = ApoToEasyEffectsFilter.at(bands[n].type);
          } catch (std::out_of_range const&) {
            curr_band_type = "Off";
          }

          g_settings_set_string(channel, band_type[n].data(), curr_band_type.c_str());

        } else {
          // If the frequency is not in the valid range, we assume the filter is
          // unsupported or disabled, so reset to default frequency and set type Off.
          g_settings_reset(channel, band_frequency[n].data());

          g_settings_set_string(channel, band_type[n].data(), "Off");
        }

        g_variant_unref(freq_variant);
        g_settings_schema_key_unref(freq_schema_key);

        // Band gain
        auto* gain_schema_key = g_settings_schema_get_key(schema, band_gain[n].data());
        auto* gain_variant = g_variant_new_double(static_cast<gdouble>(bands[n].gain));

        if (g_settings_schema_key_range_check(gain_schema_key, gain_variant) != 0) {
          g_settings_set_double(channel, band_gain[n].data(), bands[n].gain);
        } else {
          g_settings_reset(channel, band_gain[n].data());
        }

        g_variant_unref(gain_variant);
        g_settings_schema_key_unref(gain_schema_key);

        // Band quality
        auto* q_schema_key = g_settings_schema_get_key(schema, band_q[n].data());
        auto* q_variant = g_variant_new_double(static_cast<gdouble>(bands[n].quality));

        if (g_settings_schema_key_range_check(q_schema_key, q_variant) != 0) {
          g_settings_set_double(channel, band_q[n].data(), bands[n].quality);
        } else {
          g_settings_reset(channel, band_q[n].data());
        }

        g_variant_unref(q_variant);
        g_settings_schema_key_unref(q_schema_key);

        // Band mode
        g_settings_set_string(channel, band_mode[n].data(), "APO (DR)");
      } else {
        g_settings_reset(channel, band_frequency[n].data());
        g_settings_set_string(channel, band_type[n].data(), "Off");
        g_settings_reset(channel, band_gain[n].data());
        g_settings_reset(channel, band_q[n].data());
        g_settings_reset(channel, band_mode[n].data());
      }

      g_settings_reset(channel, band_width[n].data());
      g_settings_reset(channel, band_slope[n].data());
      g_settings_reset(channel, band_mute[n].data());
      g_settings_reset(channel, band_solo[n].data());
    }
  }

  g_settings_schema_unref(schema);

  return true;
}

void on_import_apo_preset_clicked(EqualizerBox* self, GtkButton* btn) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto* dialog = gtk_file_dialog_new();

  gtk_file_dialog_set_title(dialog, _("Import APO Preset File"));
  gtk_file_dialog_set_accept_label(dialog, _("Open"));

  GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_add_mime_type(filter, "text/plain");
  gtk_file_filter_set_name(filter, _("APO Presets"));

  g_list_store_append(filters, filter);

  g_object_unref(filter);

  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

  g_object_unref(filters);

  gtk_file_dialog_open(
      dialog, active_window, nullptr,
      +[](GObject* source_object, GAsyncResult* result, gpointer user_data) {
        auto* self = static_cast<EqualizerBox*>(user_data);
        auto* dialog = GTK_FILE_DIALOG(source_object);

        auto* file = gtk_file_dialog_open_finish(dialog, result, nullptr);

        if (file != nullptr) {
          auto* path = g_file_get_path(file);

          if (!import_apo_preset(self, path)) {
            // notify error on preset loading
            ui::show_fixed_toast(
                self->toast_overlay,
                _("APO Preset Not Loaded. File Format May Be Not Supported. Please Check Its Content."));
          }

          g_free(path);

          g_object_unref(file);
        }
      },
      self);
}

auto export_apo_preset(EqualizerBox* self, GFile* file) {
  GFileOutputStream* output_stream = g_file_replace(file, nullptr, false, G_FILE_CREATE_NONE, nullptr, nullptr);

  if (output_stream == nullptr) {
    return false;
  }

  std::ostringstream write_buffer;
  const double preamp = g_settings_get_double(self->settings, "input-gain");

  write_buffer << "Preamp: " << util::to_string(preamp) << " db"
               << "\n";

  const auto nbands = gtk_spin_button_get_value_as_int(self->nbands);

  for (int i = 0, k = 1; i < nbands; ++i) {
    const auto curr_band_type = util::gsettings_get_string(self->settings_left, band_type[i].data());

    if (curr_band_type == "Off") {
      // Skip disabled filters, we only export active ones.
      continue;
    }

    APO_Band apo_band;

    try {
      apo_band.type = EasyEffectsToApoFilter.at(curr_band_type);
    } catch (std::out_of_range const&) {
      // LSP filters not supported in APO defaults to Peak/Bell (see ticket #3882)
      apo_band.type = "PK";
    }

    apo_band.freq = g_settings_get_double(self->settings_left, band_frequency[i].data());
    apo_band.gain = g_settings_get_double(self->settings_left, band_gain[i].data());
    apo_band.quality = g_settings_get_double(self->settings_left, band_q[i].data());

    write_buffer << "Filter " << util::to_string(k++) << ": ON " << apo_band.type << " Fc "
                 << util::to_string(apo_band.freq) << " Hz";

    if (curr_band_type == "Bell" || curr_band_type == "Lo-shelf" || curr_band_type == "Hi-shelf") {
      // According to APO config documentation, gain value should only be defined
      // for Peak, Low-shelf and High-shelf filters.
      write_buffer << " Gain " << util::to_string(apo_band.gain) << " dB";
    }

    write_buffer << " Q " << util::to_string(apo_band.quality) << "\n";
  }

  if (g_output_stream_write(G_OUTPUT_STREAM(output_stream), write_buffer.str().c_str(), write_buffer.str().size(),
                            nullptr, nullptr) == -1) {
    return false;
  }

  return g_output_stream_close(G_OUTPUT_STREAM(output_stream), nullptr, nullptr) != 0;
}

void on_export_apo_preset_clicked(EqualizerBox* self, GtkButton* btn) {
  if (static_cast<bool>(g_settings_get_boolean(self->settings, "split-channels"))) {
    ui::show_fixed_toast(self->toast_overlay, _("Split channels not yet supported when exporting APO presets."));
    return;
  }

  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto* dialog = gtk_file_dialog_new();

  gtk_file_dialog_set_title(dialog, _("Export EqualizerAPO Preset File"));
  gtk_file_dialog_set_accept_label(dialog, _("Save"));

  GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_add_mime_type(filter, "text/plain");
  gtk_file_filter_set_name(filter, _("EqualizerAPO Presets"));

  g_list_store_append(filters, filter);

  g_object_unref(filter);

  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

  g_object_unref(filters);

  gtk_file_dialog_save(
      dialog, active_window, nullptr,
      +[](GObject* source_object, GAsyncResult* result, gpointer user_data) {
        auto* self = static_cast<EqualizerBox*>(user_data);
        auto* dialog = GTK_FILE_DIALOG(source_object);

        if (auto* file = gtk_file_dialog_save_finish(dialog, result, nullptr); file != nullptr) {
          export_apo_preset(self, file);
          g_object_unref(file);
        }
      },
      self);
}

// ### End APO Preset Section ###

// ### GraphicEQ Section ###

auto parse_graphiceq_config(const std::string& str, std::vector<struct GraphicEQ_Band>& bands) -> bool {
  std::smatch full_match;

  // The first parsing stage is to ensure the given string contains a
  // substring corresponding to the GraphicEQ format reported in the documentation:
  // https://sourceforge.net/p/equalizerapo/wiki/Configuration%20reference/#graphiceq-since-version-10

  // In order to do it, the following regular expression is used:
  static const auto re_geq = std::regex(
      R"(graphiceq\s*:((?:\s*\d+(?:,\d+)?(?:\.\d+)?\s+[+-]?\d+(?:\.\d+)?[ \t]*(?:;|$))+))", std::regex::icase);

  // That regex is quite permissive since:
  // - It's case insensitive;
  // - Gain values can be signed (with leading +/-);
  // - Frequency values can use a comma as thousand separator.

  // Note that the last class does not include the newline as whitespaces to allow
  // matching the `$` as the end of line (not needed in this case, but it will also
  // work if the input string will be multiline in the future).
  // This ensures the last band is captured with or without the final `;`.
  // The regex has been tested at https://regex101.com/r/JRwf4G/1

  std::regex_search(str, full_match, re_geq);

  // The regex captures the full match and a group related to the sequential bands.
  if (full_match.size() != 2U) {
    return false;
  }

  // Save the substring with all the bands and use it to extract the values.
  // It can't be const because it's used to store the sub-sequential strings
  // from the match_result class with suffix(). See the following while loop.
  auto bands_substr = full_match.str(1);

  // Couldn't we extract the values in one only regex checking also the GraphicEQ format?
  // No, there's no way. Even with Perl Compatible Regex (PCRE) checking the whole format
  // and capturing the values will return only the last repeated group (the last band),
  // but we need all of them.
  std::smatch band_match;
  static const auto re_geq_band = std::regex(R"((\d+(?:,\d+)?(?:\.\d+)?)\s+([+-]?\d+(?:\.\d+)?))");

  // C++ regex does not support the global PCRE flag, so we need to repeat the search in a loop.
  while (std::regex_search(bands_substr, band_match, re_geq_band)) {
    // The size of the match should be 3:
    // The full match with two additional groups (frequency and gain value).
    if (band_match.size() != 3U) {
      break;
    }

    struct GraphicEQ_Band band;

    // Extract frequency. It could have a comma as thousands separator
    // to be removed for the correct float conversion.
    const auto freq_str = std::regex_replace(band_match.str(1), std::regex(","), "");
    util::str_to_num(freq_str, band.freq);

    // Extract gain.
    const auto gain_str = band_match.str(2);
    util::str_to_num(gain_str, band.gain);

    // Push the band into the vector.
    bands.push_back(band);

    // Save the sub-sequential string, so the regex can return the match
    // for the following band (if existing).
    bands_substr = band_match.suffix().str();
  }

  return !bands.empty();
}

auto import_graphiceq_preset(EqualizerBox* self, const std::string& file_path) -> bool {
  std::filesystem::path p{file_path};

  if (!std::filesystem::is_regular_file(p)) {
    return false;
  }

  std::ifstream eq_file;
  eq_file.open(p.c_str());

  std::vector<struct GraphicEQ_Band> bands;

  if (const auto re = std::regex(R"(^[ \t]*#)"); eq_file.is_open()) {
    for (std::string line; std::getline(eq_file, line);) {
      if (std::regex_search(line, re)) {  // Avoid commented lines
        continue;
      }
      if (parse_graphiceq_config(line, bands)) {
        break;
      }
    }
  }

  eq_file.close();

  if (bands.empty()) {
    return false;
  }

  /* Sort bands by freq is made by user through Equalizer::sort_bands()
  std::ranges::stable_sort(bands, {}, &GraphicEQ_Band::freq); */

  const auto& max_bands = self->data->equalizer->max_bands;

  // Reset preamp
  g_settings_reset(self->settings, "input-gain");

  // Apply GraphicEQ parameters obtained
  g_settings_set_int(self->settings, "num-bands",
                     static_cast<int>(std::min(static_cast<uint>(bands.size()), max_bands)));

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

  // Retrieve GSettingsSchema* for range check
  GSettingsSchema* schema = nullptr;

  g_object_get(self->settings_left, "settings-schema", &schema, nullptr);

  if (schema == nullptr) {
    return false;
  }

  // Apply GraphicEQ parameters obtained for each band
  for (uint n = 0U, geq_bands = bands.size(); n < max_bands; n++) {
    for (auto* channel : settings_channels) {
      if (n < geq_bands) {
        // Band frequency and type
        auto* freq_schema_key = g_settings_schema_get_key(schema, band_frequency[n].data());
        auto* freq_variant = g_variant_new_double(static_cast<gdouble>(bands[n].freq));

        if (g_settings_schema_key_range_check(freq_schema_key, freq_variant) != 0) {
          g_settings_set_double(channel, band_frequency[n].data(), bands[n].freq);

          g_settings_set_string(channel, band_type[n].data(), "Bell");
        } else {
          // If the frequency is not in the valid range, we assume the filter is
          // unsupported or disabled, so reset to default frequency and set type Off.
          g_settings_reset(channel, band_frequency[n].data());

          g_settings_set_string(channel, band_type[n].data(), "Off");
        }

        g_variant_unref(freq_variant);
        g_settings_schema_key_unref(freq_schema_key);

        // Band gain
        auto* gain_schema_key = g_settings_schema_get_key(schema, band_gain[n].data());
        auto* gain_variant = g_variant_new_double(static_cast<gdouble>(bands[n].gain));

        if (g_settings_schema_key_range_check(gain_schema_key, gain_variant) != 0) {
          g_settings_set_double(channel, band_gain[n].data(), bands[n].gain);
        } else {
          g_settings_reset(channel, band_gain[n].data());
        }

        g_variant_unref(gain_variant);
        g_settings_schema_key_unref(gain_schema_key);
      } else {
        g_settings_reset(channel, band_frequency[n].data());
        g_settings_reset(channel, band_gain[n].data());
        g_settings_set_string(channel, band_type[n].data(), "Off");
      }

      g_settings_reset(channel, band_mode[n].data());
      g_settings_reset(channel, band_q[n].data());
      g_settings_reset(channel, band_width[n].data());
      g_settings_reset(channel, band_slope[n].data());
      g_settings_reset(channel, band_solo[n].data());
      g_settings_reset(channel, band_mute[n].data());
    }
  }

  g_settings_schema_unref(schema);

  return true;
}

void on_import_geq_preset_clicked(EqualizerBox* self, GtkButton* btn) {
  auto* active_window = gtk_application_get_active_window(GTK_APPLICATION(self->data->application));

  auto* dialog = gtk_file_dialog_new();

  gtk_file_dialog_set_title(dialog, _("Import GraphicEQ Preset File"));
  gtk_file_dialog_set_accept_label(dialog, _("Open"));

  GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);

  auto* filter = gtk_file_filter_new();

  gtk_file_filter_add_mime_type(filter, "text/plain");
  gtk_file_filter_set_name(filter, _("GraphicEQ Presets"));

  g_list_store_append(filters, filter);

  g_object_unref(filter);

  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

  g_object_unref(filters);

  gtk_file_dialog_open(
      dialog, active_window, nullptr,
      +[](GObject* source_object, GAsyncResult* result, gpointer user_data) {
        auto* self = static_cast<EqualizerBox*>(user_data);
        auto* dialog = GTK_FILE_DIALOG(source_object);

        auto* file = gtk_file_dialog_open_finish(dialog, result, nullptr);

        if (file != nullptr) {
          auto* path = g_file_get_path(file);

          if (!import_graphiceq_preset(self, path)) {
            // notify error on preset loading
            ui::show_fixed_toast(
                self->toast_overlay,
                _("GraphicEQ Preset Not Loaded. File Format May Be Not Supported. Please Check Its Content."));
          }

          g_free(path);

          g_object_unref(file);
        }
      },
      self);
}

// ### End GraphicEQ Section ###

auto sort_band_widgets(EqualizerBox* self, const int nbands, GSettings* settings, const bool& sort_by_freq)
    -> std::vector<std::string> {
  std::vector<int> list(nbands);

  std::iota(list.begin(), list.end(), 0);

  if (sort_by_freq) {
    std::ranges::sort(list, [=](const int& a, const int& b) {
      const auto freq_a = g_settings_get_double(settings, band_frequency[a].data());

      const auto freq_b = g_settings_get_double(settings, band_frequency[b].data());

      return freq_a < freq_b;
    });
  }

  std::vector<std::string> output;

  output.reserve(nbands);

  for (int n = 0; n < nbands; n++) {
    output.push_back(util::to_string(list[n]));
  }

  return output;
}

void build_all_bands(EqualizerBox* self, const bool& sort_by_freq = false) {
  const auto split = g_settings_get_boolean(self->settings, "split-channels") != 0;

  const auto nbands = g_settings_get_int(self->settings, "num-bands");

  auto list = sort_band_widgets(self, nbands, self->settings_left, sort_by_freq);

  gtk_string_list_splice(self->string_list_left, 0, g_list_model_get_n_items(G_LIST_MODEL(self->string_list_left)),
                         util::make_gchar_pointer_vector(list).data());

  if (split) {
    list = sort_band_widgets(self, nbands, self->settings_right, sort_by_freq);

    gtk_string_list_splice(self->string_list_right, 0, g_list_model_get_n_items(G_LIST_MODEL(self->string_list_right)),
                           util::make_gchar_pointer_vector(list).data());
  }
}

void on_sort_bands(EqualizerBox* self, GtkButton* btn) {
  self->data->equalizer->sort_bands();
}

template <Channel channel>
void setup_listview(EqualizerBox* self) {
  auto* factory = gtk_signal_list_item_factory_new();

  // setting the factory callbacks

  g_signal_connect(factory, "setup",
                   G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, EqualizerBox* self) {
                     auto* band_box = ui::equalizer_band_box::create();

                     gtk_list_item_set_activatable(item, 0);
                     gtk_list_item_set_child(item, GTK_WIDGET(band_box));

                     g_object_set_data(G_OBJECT(item), "band-box", band_box);

                     if constexpr (channel == Channel::left) {
                       ui::equalizer_band_box::setup(band_box, self->settings_left);
                     } else if constexpr (channel == Channel::right) {
                       ui::equalizer_band_box::setup(band_box, self->settings_right);
                     }
                   }),
                   self);

  g_signal_connect(
      factory, "bind", G_CALLBACK(+[](GtkSignalListItemFactory* factory, GtkListItem* item, EqualizerBox* self) {
        auto* band_box =
            static_cast<ui::equalizer_band_box::EqualizerBandBox*>(g_object_get_data(G_OBJECT(item), "band-box"));

        auto* child_item = gtk_list_item_get_item(item);
        auto* band_id_str = gtk_string_object_get_string(GTK_STRING_OBJECT(child_item));

        int band_id = 0;

        util::str_to_num(band_id_str, band_id);

        ui::equalizer_band_box::bind(band_box, band_id);
      }),
      self);

  if constexpr (channel == Channel::left) {
    gtk_list_view_set_factory(self->listview_left, factory);
  } else if constexpr (channel == Channel::right) {
    gtk_list_view_set_factory(self->listview_right, factory);
  }

  g_object_unref(factory);
}

void setup(EqualizerBox* self,
           std::shared_ptr<Equalizer> equalizer,
           const std::string& schema_path,
           app::Application* application) {
  self->data->equalizer = equalizer;

  self->data->application = application;

  auto serial = get_new_filter_serial();

  self->data->serial = serial;

  g_object_set_data(G_OBJECT(self), "serial", GUINT_TO_POINTER(serial));

  set_ignore_filter_idle_add(serial, false);

  self->settings = g_settings_new_with_path(tags::schema::equalizer::id, schema_path.c_str());

  self->settings_left =
      g_settings_new_with_path(tags::schema::equalizer::channel_id, (schema_path + "leftchannel/").c_str());

  self->settings_right =
      g_settings_new_with_path(tags::schema::equalizer::channel_id, (schema_path + "rightchannel/").c_str());

  equalizer->set_post_messages(true);

  setup_listview<Channel::left>(self);
  setup_listview<Channel::right>(self);

  build_all_bands(self);

  self->data->connections.push_back(equalizer->input_level.connect([=](const float left, const float right) {
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

  self->data->connections.push_back(equalizer->output_level.connect([=](const float left, const float right) {
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

  gtk_label_set_text(self->plugin_credit, ui::get_plugin_credit_translated(self->data->equalizer->package).c_str());

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);

  g_settings_bind(self->settings, "num-bands", gtk_spin_button_get_adjustment(self->nbands), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "split-channels", self->split_channels, "active", G_SETTINGS_BIND_DEFAULT);

  ui::gsettings_bind_enum_to_combo_widget(self->settings, "mode", self->mode);

  g_settings_bind(self->settings, "balance", gtk_spin_button_get_adjustment(self->balance), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "pitch-left", gtk_spin_button_get_adjustment(self->pitch_left), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(self->settings, "pitch-right", gtk_spin_button_get_adjustment(self->pitch_right), "value",
                  G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(ui::get_global_app_settings(), "show-native-plugin-ui", self->show_native_ui, "visible",
                  G_SETTINGS_BIND_DEFAULT);

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

  self->data->equalizer->close_native_ui();

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
  g_object_unref(self->settings_left);
  g_object_unref(self->settings_right);

  util::debug("disposed");

  G_OBJECT_CLASS(equalizer_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_EQUALIZER_BOX(object);

  delete self->data;

  util::debug("finalized");

  G_OBJECT_CLASS(equalizer_box_parent_class)->finalize(object);
}

void equalizer_box_class_init(EqualizerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::equalizer_ui);

  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, toast_overlay);
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
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, plugin_credit);

  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, stack);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, listview_left);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, listview_right);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, string_list_left);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, string_list_right);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, nbands);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, mode);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, split_channels);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, balance);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, pitch_left);
  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, pitch_right);

  gtk_widget_class_bind_template_child(widget_class, EqualizerBox, show_native_ui);

  gtk_widget_class_bind_template_callback(widget_class, on_reset);

  gtk_widget_class_bind_template_callback(widget_class, on_show_native_window);
  gtk_widget_class_bind_template_callback(widget_class, on_flat_response);
  gtk_widget_class_bind_template_callback(widget_class, on_calculate_frequencies);
  gtk_widget_class_bind_template_callback(widget_class, on_sort_bands);
  gtk_widget_class_bind_template_callback(widget_class, on_import_apo_preset_clicked);
  gtk_widget_class_bind_template_callback(widget_class, on_import_geq_preset_clicked);
  gtk_widget_class_bind_template_callback(widget_class, on_export_apo_preset_clicked);
}

void equalizer_box_init(EqualizerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_spinbuttons<"%">(self->balance);

  prepare_spinbuttons<"st">(self->pitch_left, self->pitch_right);

  prepare_scales<"dB">(self->input_gain, self->output_gain);
}

auto create() -> EqualizerBox* {
  return static_cast<EqualizerBox*>(g_object_new(EE_TYPE_EQUALIZER_BOX, nullptr));
}

}  // namespace ui::equalizer_box
