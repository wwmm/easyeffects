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

#include "tags_plugin_name.hpp"

namespace {

const auto id_regex = std::regex(R"(#(\d++)$)");

}

namespace tags::plugin_name {

auto get_translated() -> std::map<std::string, std::string> {
  std::map<std::string, std::string> translated = {{autogain, _("Autogain")},
                                                   {bass_enhancer, _("Bass Enhancer")},
                                                   {bass_loudness, _("Bass Loudness")},
                                                   {compressor, _("Compressor")},
                                                   {convolver, _("Convolver")},
                                                   {crossfeed, _("Crossfeed")},
                                                   {crystalizer, _("Crystalizer")},
                                                   {deesser, _("Deesser")},
                                                   {delay, _("Delay")},
                                                   {echo_canceller, _("Echo Canceller")},
                                                   {equalizer, _("Equalizer")},
                                                   {exciter, _("Exciter")},
                                                   {filter, _("Filter")},
                                                   {gate, _("Gate")},
                                                   {limiter, _("Limiter")},
                                                   {loudness, _("Loudness")},
                                                   {maximizer, _("Maximizer")},
                                                   {multiband_compressor, _("Multiband Compressor")},
                                                   {multiband_gate, _("Multiband Gate")},
                                                   {pitch, _("Pitch")},
                                                   {reverb, _("Reverberation")},
                                                   {rnnoise, _("Noise Reduction")},
                                                   {speex, _("Speech Processor")},
                                                   {stereo_tools, _("Stereo Tools")}};

  return translated;
}

auto get_base_name(std::string_view name) -> std::string {
  if (name.starts_with(tags::plugin_name::autogain)) {
    return tags::plugin_name::autogain;
  }

  if (name.starts_with(tags::plugin_name::bass_enhancer)) {
    return tags::plugin_name::bass_enhancer;
  }

  if (name.starts_with(tags::plugin_name::bass_loudness)) {
    return tags::plugin_name::bass_loudness;
  }

  if (name.starts_with(tags::plugin_name::compressor)) {
    return tags::plugin_name::compressor;
  }

  if (name.starts_with(tags::plugin_name::convolver)) {
    return tags::plugin_name::convolver;
  }

  if (name.starts_with(tags::plugin_name::crossfeed)) {
    return tags::plugin_name::crossfeed;
  }

  if (name.starts_with(tags::plugin_name::crystalizer)) {
    return tags::plugin_name::crystalizer;
  }

  if (name.starts_with(tags::plugin_name::deesser)) {
    return tags::plugin_name::deesser;
  }

  if (name.starts_with(tags::plugin_name::delay)) {
    return tags::plugin_name::delay;
  }

  if (name.starts_with(tags::plugin_name::echo_canceller)) {
    return tags::plugin_name::echo_canceller;
  }

  if (name.starts_with(tags::plugin_name::equalizer)) {
    return tags::plugin_name::equalizer;
  }

  if (name.starts_with(tags::plugin_name::exciter)) {
    return tags::plugin_name::exciter;
  }

  if (name.starts_with(tags::plugin_name::filter)) {
    return tags::plugin_name::filter;
  }

  if (name.starts_with(tags::plugin_name::gate)) {
    return tags::plugin_name::gate;
  }

  if (name.starts_with(tags::plugin_name::limiter)) {
    return tags::plugin_name::limiter;
  }

  if (name.starts_with(tags::plugin_name::loudness)) {
    return tags::plugin_name::loudness;
  }

  if (name.starts_with(tags::plugin_name::maximizer)) {
    return tags::plugin_name::maximizer;
  }

  if (name.starts_with(tags::plugin_name::multiband_compressor)) {
    return tags::plugin_name::multiband_compressor;
  }

  if (name.starts_with(tags::plugin_name::multiband_gate)) {
    return tags::plugin_name::multiband_gate;
  }

  if (name.starts_with(tags::plugin_name::pitch)) {
    return tags::plugin_name::pitch;
  }

  if (name.starts_with(tags::plugin_name::reverb)) {
    return tags::plugin_name::reverb;
  }

  if (name.starts_with(tags::plugin_name::rnnoise)) {
    return tags::plugin_name::rnnoise;
  }

  if (name.starts_with(tags::plugin_name::speex)) {
    return tags::plugin_name::speex;
  }

  if (name.starts_with(tags::plugin_name::stereo_tools)) {
    return tags::plugin_name::stereo_tools;
  }

  return "";
}

auto get_id(const std::string& name) -> uint {
  std::smatch matches;

  std::regex_search(name, matches, id_regex);

  if (matches.size() != 2) {
    return 0;
  }

  if (uint id = 0; util::str_to_num(matches[1], id)) {
    return id;
  }

  return 0;
}

}  // namespace tags::plugin_name
