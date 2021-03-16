#ifndef PLUGIN_NAME_HPP
#define PLUGIN_NAME_HPP

#include <array>

namespace plugin_name {

constexpr auto autogain = "autogain";

constexpr auto bass_enhancer = "bass_enhancer";

constexpr auto compressor = "compressor";

constexpr auto convolver = "convolver";

constexpr auto crossfeed = "crossfeed";

constexpr auto crystalizer = "crystalizer";

constexpr auto deesser = "deesser";

constexpr auto delay = "delay";

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

constexpr auto webrtc = "webrtc";

constexpr std::array<const char* const, 2> list{autogain, bass_enhancer};

}  // namespace plugin_name

#endif