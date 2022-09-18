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

namespace tags::pipewire {

inline constexpr auto ee_source_name = "easyeffects_source";

inline constexpr auto ee_sink_name = "easyeffects_sink";

}  // namespace tags::pipewire

namespace tags::pipewire::media_class {

inline constexpr auto device = "Audio/Device";

inline constexpr auto sink = "Audio/Sink";

inline constexpr auto source = "Audio/Source";

inline constexpr auto virtual_source = "Audio/Source/Virtual";

inline constexpr auto input_stream = "Stream/Input/Audio";

inline constexpr auto output_stream = "Stream/Output/Audio";

}  // namespace tags::pipewire::media_class

namespace tags::pipewire::media_role {

inline constexpr auto dsp = "DSP";

}
