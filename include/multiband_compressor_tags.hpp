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

constexpr char band_compression_mode[][19] = {{"compression-mode0"}, {"compression-mode1"}, {"compression-mode2"},
                                              {"compression-mode3"}, {"compression-mode4"}, {"compression-mode5"},
                                              {"compression-mode6"}, {"compression-mode7"}};

constexpr char band_compressor_enable[][20] = {{"compressor-enable0"}, {"compressor-enable1"}, {"compressor-enable2"},
                                               {"compressor-enable3"}, {"compressor-enable4"}, {"compressor-enable5"},
                                               {"compressor-enable6"}, {"compressor-enable7"}};

constexpr char band_split_frequency[][18] = {{"split-frequency0"}, {"split-frequency1"}, {"split-frequency2"},
                                             {"split-frequency3"}, {"split-frequency4"}, {"split-frequency5"},
                                             {"split-frequency6"}, {"split-frequency7"}};

constexpr char band_mute[][7] = {{"mute0"}, {"mute1"}, {"mute2"}, {"mute3"},
                                 {"mute4"}, {"mute5"}, {"mute6"}, {"mute7"}};

constexpr char band_solo[][7] = {{"solo0"}, {"solo1"}, {"solo2"}, {"solo3"},
                                 {"solo4"}, {"solo5"}, {"solo6"}, {"solo7"}};

constexpr char band_lowcut_filter[][33] = {{"sidechain-custom-lowcut-filter0"}, {"sidechain-custom-lowcut-filter1"},
                                           {"sidechain-custom-lowcut-filter2"}, {"sidechain-custom-lowcut-filter3"},
                                           {"sidechain-custom-lowcut-filter4"}, {"sidechain-custom-lowcut-filter5"},
                                           {"sidechain-custom-lowcut-filter6"}, {"sidechain-custom-lowcut-filter7"}};

constexpr char band_highcut_filter[][34] = {{"sidechain-custom-highcut-filter0"}, {"sidechain-custom-highcut-filter1"},
                                            {"sidechain-custom-highcut-filter2"}, {"sidechain-custom-highcut-filter3"},
                                            {"sidechain-custom-highcut-filter4"}, {"sidechain-custom-highcut-filter5"},
                                            {"sidechain-custom-highcut-filter6"}, {"sidechain-custom-highcut-filter7"}};

constexpr char band_external_sidechain[][21] = {
    {"external-sidechain0"}, {"external-sidechain1"}, {"external-sidechain2"}, {"external-sidechain3"},
    {"external-sidechain4"}, {"external-sidechain5"}, {"external-sidechain6"}, {"external-sidechain7"}};

constexpr char band_sidechain_mode[][17] = {{"sidechain-mode0"}, {"sidechain-mode1"}, {"sidechain-mode2"},
                                            {"sidechain-mode3"}, {"sidechain-mode4"}, {"sidechain-mode5"},
                                            {"sidechain-mode6"}, {"sidechain-mode7"}};

constexpr char band_sidechain_source[][19] = {{"sidechain-source0"}, {"sidechain-source1"}, {"sidechain-source2"},
                                              {"sidechain-source3"}, {"sidechain-source4"}, {"sidechain-source5"},
                                              {"sidechain-source6"}, {"sidechain-source7"}};

constexpr char band_lowcut_filter_frequency[][29] = {{"sidechain-lowcut-frequency0"}, {"sidechain-lowcut-frequency1"},
                                                     {"sidechain-lowcut-frequency2"}, {"sidechain-lowcut-frequency3"},
                                                     {"sidechain-lowcut-frequency4"}, {"sidechain-lowcut-frequency5"},
                                                     {"sidechain-lowcut-frequency6"}, {"sidechain-lowcut-frequency7"}};

constexpr char band_highcut_filter_frequency[][30] = {
    {"sidechain-highcut-frequency0"}, {"sidechain-highcut-frequency1"}, {"sidechain-highcut-frequency2"},
    {"sidechain-highcut-frequency3"}, {"sidechain-highcut-frequency4"}, {"sidechain-highcut-frequency5"},
    {"sidechain-highcut-frequency6"}, {"sidechain-highcut-frequency7"}};

constexpr char band_attack_time[][14] = {{"attack-time0"}, {"attack-time1"}, {"attack-time2"}, {"attack-time3"},
                                         {"attack-time4"}, {"attack-time5"}, {"attack-time6"}, {"attack-time7"}};

constexpr char band_attack_threshold[][19] = {{"attack-threshold0"}, {"attack-threshold1"}, {"attack-threshold2"},
                                              {"attack-threshold3"}, {"attack-threshold4"}, {"attack-threshold5"},
                                              {"attack-threshold6"}, {"attack-threshold7"}};

constexpr char band_release_time[][15] = {{"release-time0"}, {"release-time1"}, {"release-time2"}, {"release-time3"},
                                          {"release-time4"}, {"release-time5"}, {"release-time6"}, {"release-time7"}};

constexpr char band_release_threshold[][20] = {{"release-threshold0"}, {"release-threshold1"}, {"release-threshold2"},
                                               {"release-threshold3"}, {"release-threshold4"}, {"release-threshold5"},
                                               {"release-threshold6"}, {"release-threshold7"}};

constexpr char band_ratio[][8] = {{"ratio0"}, {"ratio1"}, {"ratio2"}, {"ratio3"},
                                  {"ratio4"}, {"ratio5"}, {"ratio6"}, {"ratio7"}};

constexpr char band_knee[][7] = {{"knee0"}, {"knee1"}, {"knee2"}, {"knee3"},
                                 {"knee4"}, {"knee5"}, {"knee6"}, {"knee7"}};

constexpr char band_makeup[][9] = {{"makeup0"}, {"makeup1"}, {"makeup2"}, {"makeup3"},
                                   {"makeup4"}, {"makeup5"}, {"makeup6"}, {"makeup7"}};

constexpr char band_sidechain_preamp[][19] = {{"sidechain-preamp0"}, {"sidechain-preamp1"}, {"sidechain-preamp2"},
                                              {"sidechain-preamp3"}, {"sidechain-preamp4"}, {"sidechain-preamp5"},
                                              {"sidechain-preamp6"}, {"sidechain-preamp7"}};

constexpr char band_sidechain_reactivity[][23] = {
    {"sidechain-reactivity0"}, {"sidechain-reactivity1"}, {"sidechain-reactivity2"}, {"sidechain-reactivity3"},
    {"sidechain-reactivity4"}, {"sidechain-reactivity5"}, {"sidechain-reactivity6"}, {"sidechain-reactivity7"}};

constexpr char band_sidechain_lookahead[][22] = {
    {"sidechain-lookahead0"}, {"sidechain-lookahead1"}, {"sidechain-lookahead2"}, {"sidechain-lookahead3"},
    {"sidechain-lookahead4"}, {"sidechain-lookahead5"}, {"sidechain-lookahead6"}, {"sidechain-lookahead7"}};

constexpr char band_boost_amount[][15] = {{"boost-amount0"}, {"boost-amount1"}, {"boost-amount2"}, {"boost-amount3"},
                                          {"boost-amount4"}, {"boost-amount5"}, {"boost-amount6"}, {"boost-amount7"}};

constexpr char band_boost_threshold[][18] = {{"boost-threshold0"}, {"boost-threshold1"}, {"boost-threshold2"},
                                             {"boost-threshold3"}, {"boost-threshold4"}, {"boost-threshold5"},
                                             {"boost-threshold6"}, {"boost-threshold7"}};

}  // namespace tags::multiband_compressor