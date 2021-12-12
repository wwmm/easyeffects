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

#pragma once

#include <array>

namespace tags::multiband_compressor {

// bands

constexpr char band_mute[][7] = {{"mute0"}, {"mute1"}, {"mute2"}, {"mute3"},
                                 {"mute4"}, {"mute5"}, {"mute6"}, {"mute7"}};

constexpr char band_compression_mode[][19] = {{"compression-mode0"}, {"compression-mode1"}, {"compression-mode2"},
                                              {"compression-mode3"}, {"compression-mode4"}, {"compression-mode5"},
                                              {"compression-mode6"}, {"compression-mode7"}};

}  // namespace tags::multiband_compressor