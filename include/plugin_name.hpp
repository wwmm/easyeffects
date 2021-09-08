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

#ifndef PLUGIN_NAME_HPP
#define PLUGIN_NAME_HPP

#include <array>

namespace plugin_name {

constexpr auto autogain = "autogain";

constexpr auto bass_enhancer = "bass_enhancer";

constexpr auto bass_loudness = "bass_loudness";

constexpr auto compressor = "compressor";

constexpr auto convolver = "convolver";

constexpr auto crossfeed = "crossfeed";

constexpr auto crystalizer = "crystalizer";

constexpr auto deesser = "deesser";

constexpr auto delay = "delay";

constexpr auto echo_canceller = "echo_canceller";

constexpr auto equalizer = "equalizer";

constexpr auto exciter = "exciter";

constexpr auto filter = "filter";

constexpr auto gate = "gate";

constexpr auto limiter = "limiter";

constexpr auto loudness = "loudness";

constexpr auto maximizer = "maximizer";

constexpr auto multiband_compressor = "multiband_compressor";

constexpr auto multiband_gate = "multiband_gate";

constexpr auto pitch = "pitch";

constexpr auto reverb = "reverb";

constexpr auto rnnoise = "rnnoise";

constexpr auto stereo_tools = "stereo_tools";

constexpr std::array<const char* const, 23U> list{
    autogain,  bass_enhancer,        bass_loudness,  compressor, convolver, crossfeed, crystalizer, deesser,
    delay,     echo_canceller,       equalizer,      exciter,    filter,    gate,      limiter,     loudness,
    maximizer, multiband_compressor, multiband_gate, pitch,      reverb,    rnnoise,   stereo_tools};

}  // namespace plugin_name

#endif
