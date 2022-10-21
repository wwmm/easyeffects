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

#pragma once

namespace tags::schema {

inline constexpr auto id_input = "com.github.wwmm.easyeffects.streaminputs";

inline constexpr auto id_output = "com.github.wwmm.easyeffects.streamoutputs";

}  // namespace tags::schema

namespace tags::schema::autogain {

inline constexpr auto id = "com.github.wwmm.easyeffects.autogain";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/autogain/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/autogain/";

}  // namespace tags::schema::autogain

namespace tags::schema::bass_enhancer {

inline constexpr auto id = "com.github.wwmm.easyeffects.bassenhancer";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/bassenhancer/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/bassenhancer/";

}  // namespace tags::schema::bass_enhancer

namespace tags::schema::bass_loudness {

inline constexpr auto id = "com.github.wwmm.easyeffects.bassloudness";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/bassloudness/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/bassloudness/";

}  // namespace tags::schema::bass_loudness

namespace tags::schema::compressor {

inline constexpr auto id = "com.github.wwmm.easyeffects.compressor";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/compressor/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/compressor/";

}  // namespace tags::schema::compressor

namespace tags::schema::convolver {

inline constexpr auto id = "com.github.wwmm.easyeffects.convolver";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/convolver/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/convolver/";

}  // namespace tags::schema::convolver

namespace tags::schema::crossfeed {

inline constexpr auto id = "com.github.wwmm.easyeffects.crossfeed";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/crossfeed/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/crossfeed/";

}  // namespace tags::schema::crossfeed

namespace tags::schema::crystalizer {

inline constexpr auto id = "com.github.wwmm.easyeffects.crystalizer";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/crystalizer/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/crystalizer/";

}  // namespace tags::schema::crystalizer

namespace tags::schema::deesser {

inline constexpr auto id = "com.github.wwmm.easyeffects.deesser";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/deesser/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/deesser/";

}  // namespace tags::schema::deesser

namespace tags::schema::delay {

inline constexpr auto id = "com.github.wwmm.easyeffects.delay";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/delay/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/delay/";

}  // namespace tags::schema::delay

namespace tags::schema::echo_canceller {

inline constexpr auto id = "com.github.wwmm.easyeffects.echocanceller";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/echocanceller/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/echocanceller/";

}  // namespace tags::schema::echo_canceller

namespace tags::schema::equalizer {

inline constexpr auto id = "com.github.wwmm.easyeffects.equalizer";

inline constexpr auto channel_id = "com.github.wwmm.easyeffects.equalizer.channel";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/equalizer/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/equalizer/";

}  // namespace tags::schema::equalizer

namespace tags::schema::exciter {

inline constexpr auto id = "com.github.wwmm.easyeffects.exciter";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/exciter/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/exciter/";

}  // namespace tags::schema::exciter

namespace tags::schema::filter {

inline constexpr auto id = "com.github.wwmm.easyeffects.filter";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/filter/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/filter/";

}  // namespace tags::schema::filter

namespace tags::schema::gate {

inline constexpr auto id = "com.github.wwmm.easyeffects.gate";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/gate/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/gate/";

}  // namespace tags::schema::gate

namespace tags::schema::limiter {

inline constexpr auto id = "com.github.wwmm.easyeffects.limiter";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/limiter/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/limiter/";

}  // namespace tags::schema::limiter

namespace tags::schema::loudness {

inline constexpr auto id = "com.github.wwmm.easyeffects.loudness";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/loudness/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/loudness/";

}  // namespace tags::schema::loudness

namespace tags::schema::maximizer {

inline constexpr auto id = "com.github.wwmm.easyeffects.maximizer";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/maximizer/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/maximizer/";

}  // namespace tags::schema::maximizer

namespace tags::schema::multiband_compressor {

inline constexpr auto id = "com.github.wwmm.easyeffects.multibandcompressor";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/multibandcompressor/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/multibandcompressor/";

}  // namespace tags::schema::multiband_compressor

namespace tags::schema::multiband_gate {

inline constexpr auto id = "com.github.wwmm.easyeffects.multibandgate";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/multibandgate/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/multibandgate/";

}  // namespace tags::schema::multiband_gate

namespace tags::schema::output_level {

inline constexpr auto id = "com.github.wwmm.easyeffects.outputlevel";

}

namespace tags::schema::pitch {

inline constexpr auto id = "com.github.wwmm.easyeffects.pitch";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/pitch/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/pitch/";

}  // namespace tags::schema::pitch

namespace tags::schema::reverb {

inline constexpr auto id = "com.github.wwmm.easyeffects.reverb";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/reverb/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/reverb/";

}  // namespace tags::schema::reverb

namespace tags::schema::rnnoise {

inline constexpr auto id = "com.github.wwmm.easyeffects.rnnoise";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/rnnoise/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/rnnoise/";

}  // namespace tags::schema::rnnoise

namespace tags::schema::speex {

inline constexpr auto id = "com.github.wwmm.easyeffects.speex";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/speex/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/speex/";

}  // namespace tags::schema::speex

namespace tags::schema::stereo_tools {

inline constexpr auto id = "com.github.wwmm.easyeffects.stereotools";

inline constexpr auto input_path = "/com/github/wwmm/easyeffects/streaminputs/stereotools/";

inline constexpr auto output_path = "/com/github/wwmm/easyeffects/streamoutputs/stereotools/";

}  // namespace tags::schema::stereo_tools

namespace tags::schema::spectrum {

inline constexpr auto id = "com.github.wwmm.easyeffects.spectrum";

}  // namespace tags::schema::spectrum