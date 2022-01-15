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

#include "plugin_name.hpp"

namespace plugin_name {

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
                                                   {stereo_tools, _("Stereo Tools")}};

  return translated;
}

}  // namespace plugin_name