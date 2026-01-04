/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "equalizer_apo.hpp"
#include <qtypes.h>
#include <QString>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"
#include "tags_equalizer.hpp"
#include "util.hpp"

namespace apo {

using namespace tags::equalizer;

static auto parse_apo_preamp(const std::string& line, double& preamp) -> bool {
  std::smatch matches;

  static const auto re_preamp = std::regex(R"(preamp\s*:\s*([+-]?\d+(?:\.\d+)?)\s*db)", std::regex::icase);

  std::regex_search(line, matches, re_preamp);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), preamp);
}

static auto parse_apo_filter_type(const std::string& line, struct APO_Band& filter) -> bool {
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
  std::ranges::transform(filter.type, filter.type.begin(), [](unsigned char c) { return std::toupper(c); });

  return !filter.type.empty();
}

static auto parse_apo_frequency(const std::string& line, struct APO_Band& filter) -> bool {
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

static auto parse_apo_gain(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_gain = std::regex(R"(gain\s+([+-]?\d+(?:\.\d+)?)\s*db)", std::regex::icase);

  std::regex_search(line, matches, re_gain);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), filter.gain);
}

static auto parse_apo_quality(const std::string& line, struct APO_Band& filter) -> bool {
  std::smatch matches;

  static const auto re_quality = std::regex(R"(q\s+(\d+(?:\.\d+)?))", std::regex::icase);

  std::regex_search(line, matches, re_quality);

  if (matches.size() != 2U) {
    return false;
  }

  return util::str_to_num(matches.str(1), filter.quality);
}

static auto parse_apo_config_line(const std::string& line, struct APO_Band& filter) -> bool {
  // Retrieve filter type.
  if (!parse_apo_filter_type(line, filter)) {
    // If we can't parse the filter type, there's something wrong in the text
    // line, so exit with false.
    return false;
  }

  // Retrieve frequency.
  // To make it more permissive, we do not exit on false here (assume default).
  parse_apo_frequency(line, filter);

  /**
   * The following has been inspired by the function
   * "para_equalizer_ui::import_rew_file(const LSPString*)"
   * inside 'lsp-plugins/src/ui/plugins/para_equalizer_ui.cpp' at
   * https://github.com/sadko4u/lsp-plugins
   *
   * Retrieve gain and/or quality parameters based on a specific filter type.
   * Calculate frequency/quality if needed.
   * If the APO filter type is different than the ones specified below,
   * it's set as "Off" and default values are assumed since
   * it may not be supported by LSP Equalizer.
   */
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
    /**
     * Notch filter
     * Q value is optional for this filter according to APO config
     * documentation, but LSP import function always sets it to 100/3.
     */
    filter.quality = 100.0F / 3.0F;
  } else if (filter.type == "AP") {
    /**
     * All-pass filter
     * Q value is mandatory for this filter according to APO config
     * documentation, but LSP import function always sets it to 0,
     * no matter which quality value the APO config has.
     */
    filter.quality = 0.0F;
  }

  return true;
}

auto import_apo_preset(db::Equalizer* settings,
                       db::EqualizerChannel* settings_left,
                       db::EqualizerChannel* settings_right,
                       const std::string& file_path) -> bool {
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

  // Sort bands by freq is made by user through Equalizer::sort_bands()
  // std::ranges::stable_sort(bands, {}, &APO_Band::freq);

  const auto max_bands = settings->getMaxValue(num_bands).value<int>();
  const auto apo_bands = static_cast<int>(bands.size());

  // Apply APO parameters obtained
  settings->setInputGain(preamp);

  std::vector<db::EqualizerChannel*> settings_channels;

  /**
   * When split channel mode is disabled, we can:
   * - Select the band settings of both channels.
   * - Set the band number according to the imported APO bands.
   *
   * In split channels mode, instead, we have to:
   * - Select only the band settings of the visible channel, because the user
   *   may want to import different APO presets for each channel.
   * - Avoid to reduce the current band number because we only import to a
   *   single channel, so we do NOT want to change the current band parameters
   *   of the other channel.
   */
  if (!settings->splitChannels()) {
    settings->setNumBands(std::min(apo_bands, max_bands));

    settings_channels.push_back(settings_left);
    settings_channels.push_back(settings_right);
  } else {
    settings->setNumBands(std::max(settings->numBands(), std::min(apo_bands, max_bands)));

    if (settings->viewLeftChannel()) {
      settings_channels.push_back(settings_left);
    } else {
      settings_channels.push_back(settings_right);
    }
  }

  // Apply APO parameters obtained for each band
  for (int n = 0U; n < max_bands; n++) {
    for (auto* channel : settings_channels) {
      if (n < apo_bands) {
        // Band frequency and type

        if (bands[n].freq >= channel->getMinValue(band_frequency[n].data()).value<float>() &&
            bands[n].freq <= channel->getMaxValue(band_frequency[n].data()).value<float>()) {
          channel->setProperty(band_frequency[n].data(), bands[n].freq);

          std::string curr_band_type;

          try {
            curr_band_type = ApoToEasyEffectsFilter.at(bands[n].type);
          } catch (std::out_of_range const&) {
            curr_band_type = "Off";
          }

          channel->setProperty(band_type[n].data(), channel->bandTypeLabels().indexOf(curr_band_type));

        } else {
          // If the frequency is not in the valid range, we assume the filter is
          // unsupported or disabled, so reset to default frequency and set type Off.
          channel->resetProperty(band_frequency[n].data());
          channel->setProperty(band_type[n].data(), channel->bandTypeLabels().indexOf("Off"));
        }

        // Band gain

        if (bands[n].gain >= channel->getMinValue(band_gain[n].data()).value<float>() &&
            bands[n].gain <= channel->getMaxValue(band_gain[n].data()).value<float>()) {
          channel->setProperty(band_gain[n].data(), bands[n].gain);

        } else {
          channel->resetProperty(band_gain[n].data());
        }

        // Band quality

        if (bands[n].quality >= channel->getMinValue(band_q[n].data()).value<float>() &&
            bands[n].quality <= channel->getMaxValue(band_q[n].data()).value<float>()) {
          channel->setProperty(band_q[n].data(), bands[n].quality);

        } else {
          channel->resetProperty(band_q[n].data());
        }

        // Band mode
        channel->setProperty(band_mode[n].data(), channel->bandModeLabels().indexOf("APO (DR)"));
      } else {
        channel->setProperty(band_type[n].data(), channel->bandTypeLabels().indexOf("Off"));
        channel->resetProperty(band_frequency[n].data());
        channel->resetProperty(band_gain[n].data());
        channel->resetProperty(band_q[n].data());
        channel->resetProperty(band_mode[n].data());
      }

      channel->resetProperty(band_width[n].data());
      channel->resetProperty(band_slope[n].data());
      channel->resetProperty(band_solo[n].data());
      channel->resetProperty(band_mute[n].data());
    }
  }

  return true;
}

// ### GraphicEQ Section ###

static auto parse_graphiceq_config(const std::string& str, std::vector<struct GraphicEQ_Band>& bands) -> bool {
  std::smatch full_match;

  /**
   * The first parsing stage is to ensure the given string contains a
   * substring corresponding to the GraphicEQ format reported in the
   * documentation:
   * https://sourceforge.net/p/equalizerapo/wiki/Configuration%20reference/#graphiceq-since-version-10
   *
   * In order to do it, the following regular expression is used:
   */
  static const auto re_geq = std::regex(
      R"(graphiceq\s*:((?:\s*\d+(?:,\d+)?(?:\.\d+)?\s+[+-]?\d+(?:\.\d+)?[ \t]*(?:;|$))+))", std::regex::icase);

  /**
   * That regex is quite permissive since:
   * - It's case insensitive;
   * - Gain values can be signed (with leading +/-);
   * - Frequency values can use a comma as thousand separator.
   *
   * Note that the last class does not include the newline as whitespaces to
   * allow matching the `$` as the end of line (not needed in this case, but it
   * will also work if the input string will be multiline in the future).
   * This ensures the last band is captured with or without the final `;`.
   * The regex has been tested at https://regex101.com/r/JRwf4G/1
   */

  std::regex_search(str, full_match, re_geq);

  // The regex captures the full match and a group related to the sequential bands.
  if (full_match.size() != 2U) {
    return false;
  }

  /**
   * Save the substring with all the bands and use it to extract the values.
   * It can't be const because it's used to store the sub-sequential strings
   * from the match_result class with suffix(). See the following while loop.
   */
  auto bands_substr = full_match.str(1);

  /**
   * Couldn't we extract the values in one only regex checking also the
   * GraphicEQ format?
   * No, there's no way. Even with Perl Compatible Regex (PCRE) checking the
   * whole format and capturing the values will return only the last repeated
   * group (the last band), but we need all of them.
   */
  std::smatch band_match;
  static const auto re_geq_band = std::regex(R"((\d+(?:,\d+)?(?:\.\d+)?)\s+([+-]?\d+(?:\.\d+)?))");

  // C++ regex does not support the global PCRE flag, so we need to repeat
  // the search in a loop.
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

auto import_graphiceq_preset(db::Equalizer* settings,
                             db::EqualizerChannel* settings_left,
                             db::EqualizerChannel* settings_right,
                             const std::string& file_path) -> bool {
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

  // Sort bands by freq is made by user through Equalizer::sort_bands()
  // std::ranges::stable_sort(bands, {}, &GraphicEQ_Band::freq);

  const auto max_bands = settings->getMaxValue(num_bands).value<int>();
  const auto geq_bands = static_cast<int>(bands.size());

  // Reset preamp
  settings->resetProperty("input-gain");

  // Apply GraphicEQ parameters obtained

  std::vector<db::EqualizerChannel*> settings_channels;

  // Whether to apply the parameters to both channels or the selected one only
  if (!settings->splitChannels()) {
    settings->setNumBands(std::min(geq_bands, max_bands));

    settings_channels.push_back(settings_left);
    settings_channels.push_back(settings_right);
  } else {
    settings->setNumBands(std::max(settings->numBands(), std::min(geq_bands, max_bands)));

    if (settings->viewLeftChannel()) {
      settings_channels.push_back(settings_left);
    } else {
      settings_channels.push_back(settings_right);
    }
  }

  // Apply GraphicEQ parameters obtained for each band
  for (int n = 0U; n < max_bands; n++) {
    for (auto* channel : settings_channels) {
      if (n < geq_bands) {
        // Band frequency and type

        if (bands[n].freq >= channel->getMinValue(band_frequency[n].data()).value<float>() &&
            bands[n].freq <= channel->getMaxValue(band_frequency[n].data()).value<float>()) {
          channel->setProperty(band_frequency[n].data(), bands[n].freq);
          channel->setProperty(band_type[n].data(), channel->bandTypeLabels().indexOf("Bell"));
        } else {
          // If the frequency is not in the valid range, we assume the filter is
          // unsupported or disabled, so reset to default frequency and set type Off.
          channel->resetProperty(band_frequency[n].data());
          channel->setProperty(band_type[n].data(), channel->bandTypeLabels().indexOf("Off"));
        }

        // Band gain

        if (bands[n].gain >= channel->getMinValue(band_gain[n].data()).value<float>() &&
            bands[n].gain <= channel->getMaxValue(band_gain[n].data()).value<float>()) {
          channel->setProperty(band_gain[n].data(), bands[n].gain);

        } else {
          channel->resetProperty(band_gain[n].data());
        }
      } else {
        channel->setProperty(band_type[n].data(), channel->bandTypeLabels().indexOf("Off"));
        channel->resetProperty(band_frequency[n].data());
        channel->resetProperty(band_gain[n].data());
      }

      channel->resetProperty(band_q[n].data());
      channel->resetProperty(band_mode[n].data());
      channel->resetProperty(band_width[n].data());
      channel->resetProperty(band_slope[n].data());
      channel->resetProperty(band_solo[n].data());
      channel->resetProperty(band_mute[n].data());
    }
  }

  return true;
}

auto export_apo_preset(db::Equalizer* settings,
                       db::EqualizerChannel* settings_left,
                       db::EqualizerChannel* settings_right,
                       const std::string& file_path) -> bool {
  std::ofstream write_buffer(file_path);

  const double preamp = settings->inputGain();

  write_buffer << "Preamp: " << util::to_string(preamp) << " db"
               << "\n";

  db::EqualizerChannel* settings_channel = nullptr;

  // Whether to export the parameters from the left or the right channel.
  if (!settings->splitChannels()) {
    settings_channel = settings_left;
  } else {
    settings_channel = settings->viewLeftChannel() ? settings_left : settings_right;
  }

  for (int i = 0, k = 1; i < settings->numBands(); ++i) {
    const auto curr_band_type =
        settings_channel->bandTypeLabels()[settings_channel->property(band_type[i].data()).value<int>()];

    if (curr_band_type == "Off") {
      // Skip disabled filters, we only export active ones.
      continue;
    }

    APO_Band apo_band;

    try {
      apo_band.type = EasyEffectsToApoFilter.at(curr_band_type.toStdString());
    } catch (std::out_of_range const&) {
      // LSP filters not supported in APO defaults to Peak/Bell (see ticket #3882)
      apo_band.type = "PK";
    }

    apo_band.freq = settings_channel->property(band_frequency[i].data()).value<float>();
    apo_band.gain = settings_channel->property(band_gain[i].data()).value<float>();
    apo_band.quality = settings_channel->property(band_q[i].data()).value<float>();

    write_buffer << "Filter " << util::to_string(k++) << ": ON " << apo_band.type << " Fc "
                 << util::to_string(apo_band.freq) << " Hz";

    if (curr_band_type == "Bell" || curr_band_type == "Lo-shelf" || curr_band_type == "Hi-shelf") {
      // According to APO config documentation, gain value should only be
      // defined for Peak, Low-shelf and High-shelf filters.
      write_buffer << " Gain " << util::to_string(apo_band.gain) << " dB";
    }

    write_buffer << " Q " << util::to_string(apo_band.quality) << "\n";
  }

  return !write_buffer.fail();
}

}  // namespace apo
