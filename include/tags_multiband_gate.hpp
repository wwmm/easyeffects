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

namespace tags::multiband_gate {

// gsettings keys tags

inline constexpr char band_enable[][14] = {{"enable-band0"}, {"enable-band1"}, {"enable-band2"}, {"enable-band3"},
                                           {"enable-band4"}, {"enable-band5"}, {"enable-band6"}, {"enable-band7"}};

inline constexpr char band_gate_enable[][20] = {{"gate-enable0"}, {"gate-enable1"}, {"gate-enable2"}, {"gate-enable3"},
                                                {"gate-enable4"}, {"gate-enable5"}, {"gate-enable6"}, {"gate-enable7"}};

inline constexpr char band_split_frequency[][18] = {{"split-frequency0"}, {"split-frequency1"}, {"split-frequency2"},
                                                    {"split-frequency3"}, {"split-frequency4"}, {"split-frequency5"},
                                                    {"split-frequency6"}, {"split-frequency7"}};

inline constexpr char band_mute[][7] = {{"mute0"}, {"mute1"}, {"mute2"}, {"mute3"},
                                        {"mute4"}, {"mute5"}, {"mute6"}, {"mute7"}};

inline constexpr char band_solo[][7] = {{"solo0"}, {"solo1"}, {"solo2"}, {"solo3"},
                                        {"solo4"}, {"solo5"}, {"solo6"}, {"solo7"}};

inline constexpr char band_lowcut_filter[][33] = {
    {"sidechain-custom-lowcut-filter0"}, {"sidechain-custom-lowcut-filter1"}, {"sidechain-custom-lowcut-filter2"},
    {"sidechain-custom-lowcut-filter3"}, {"sidechain-custom-lowcut-filter4"}, {"sidechain-custom-lowcut-filter5"},
    {"sidechain-custom-lowcut-filter6"}, {"sidechain-custom-lowcut-filter7"}};

inline constexpr char band_highcut_filter[][34] = {
    {"sidechain-custom-highcut-filter0"}, {"sidechain-custom-highcut-filter1"}, {"sidechain-custom-highcut-filter2"},
    {"sidechain-custom-highcut-filter3"}, {"sidechain-custom-highcut-filter4"}, {"sidechain-custom-highcut-filter5"},
    {"sidechain-custom-highcut-filter6"}, {"sidechain-custom-highcut-filter7"}};

inline constexpr char band_external_sidechain[][21] = {
    {"external-sidechain0"}, {"external-sidechain1"}, {"external-sidechain2"}, {"external-sidechain3"},
    {"external-sidechain4"}, {"external-sidechain5"}, {"external-sidechain6"}, {"external-sidechain7"}};

inline constexpr char band_sidechain_mode[][17] = {{"sidechain-mode0"}, {"sidechain-mode1"}, {"sidechain-mode2"},
                                                   {"sidechain-mode3"}, {"sidechain-mode4"}, {"sidechain-mode5"},
                                                   {"sidechain-mode6"}, {"sidechain-mode7"}};

inline constexpr char band_sidechain_source[][19] = {
    {"sidechain-source0"}, {"sidechain-source1"}, {"sidechain-source2"}, {"sidechain-source3"},
    {"sidechain-source4"}, {"sidechain-source5"}, {"sidechain-source6"}, {"sidechain-source7"}};

inline constexpr char band_lowcut_filter_frequency[][29] = {
    {"sidechain-lowcut-frequency0"}, {"sidechain-lowcut-frequency1"}, {"sidechain-lowcut-frequency2"},
    {"sidechain-lowcut-frequency3"}, {"sidechain-lowcut-frequency4"}, {"sidechain-lowcut-frequency5"},
    {"sidechain-lowcut-frequency6"}, {"sidechain-lowcut-frequency7"}};

inline constexpr char band_highcut_filter_frequency[][30] = {
    {"sidechain-highcut-frequency0"}, {"sidechain-highcut-frequency1"}, {"sidechain-highcut-frequency2"},
    {"sidechain-highcut-frequency3"}, {"sidechain-highcut-frequency4"}, {"sidechain-highcut-frequency5"},
    {"sidechain-highcut-frequency6"}, {"sidechain-highcut-frequency7"}};

inline constexpr char band_attack_time[][14] = {{"attack-time0"}, {"attack-time1"}, {"attack-time2"}, {"attack-time3"},
                                                {"attack-time4"}, {"attack-time5"}, {"attack-time6"}, {"attack-time7"}};

inline constexpr char band_release_time[][15] = {{"release-time0"}, {"release-time1"}, {"release-time2"},
                                                 {"release-time3"}, {"release-time4"}, {"release-time5"},
                                                 {"release-time6"}, {"release-time7"}};

inline constexpr char band_hysteresis[][19] = {{"hysteresis0"}, {"hysteresis1"}, {"hysteresis2"}, {"hysteresis3"},
                                               {"hysteresis4"}, {"hysteresis5"}, {"hysteresis6"}, {"hysteresis7"}};

inline constexpr char band_hysteresis_threshold[][20] = {
    {"hysteresis-threshold0"}, {"hysteresis-threshold1"}, {"hysteresis-threshold2"}, {"hysteresis-threshold3"},
    {"hysteresis-threshold4"}, {"hysteresis-threshold5"}, {"hysteresis-threshold6"}, {"hysteresis-threshold7"}};

inline constexpr char band_hysteresis_zone[][20] = {{"hysteresis-zone0"}, {"hysteresis-zone1"}, {"hysteresis-zone2"},
                                                    {"hysteresis-zone3"}, {"hysteresis-zone4"}, {"hysteresis-zone5"},
                                                    {"hysteresis-zone6"}, {"hysteresis-zone7"}};

inline constexpr char band_curve_threshold[][20] = {{"curve-threshold0"}, {"curve-threshold1"}, {"curve-threshold2"},
                                                    {"curve-threshold3"}, {"curve-threshold4"}, {"curve-threshold5"},
                                                    {"curve-threshold6"}, {"curve-threshold7"}};

inline constexpr char band_curve_zone[][20] = {{"curve-zone0"}, {"curve-zone1"}, {"curve-zone2"}, {"curve-zone3"},
                                               {"curve-zone4"}, {"curve-zone5"}, {"curve-zone6"}, {"curve-zone7"}};

inline constexpr char band_reduction[][7] = {{"reduction0"}, {"reduction1"}, {"reduction2"}, {"reduction3"},
                                             {"reduction4"}, {"reduction5"}, {"reduction6"}, {"reduction7"}};

inline constexpr char band_makeup[][9] = {{"makeup0"}, {"makeup1"}, {"makeup2"}, {"makeup3"},
                                          {"makeup4"}, {"makeup5"}, {"makeup6"}, {"makeup7"}};

inline constexpr char band_sidechain_preamp[][19] = {
    {"sidechain-preamp0"}, {"sidechain-preamp1"}, {"sidechain-preamp2"}, {"sidechain-preamp3"},
    {"sidechain-preamp4"}, {"sidechain-preamp5"}, {"sidechain-preamp6"}, {"sidechain-preamp7"}};

inline constexpr char band_sidechain_reactivity[][23] = {
    {"sidechain-reactivity0"}, {"sidechain-reactivity1"}, {"sidechain-reactivity2"}, {"sidechain-reactivity3"},
    {"sidechain-reactivity4"}, {"sidechain-reactivity5"}, {"sidechain-reactivity6"}, {"sidechain-reactivity7"}};

inline constexpr char band_sidechain_lookahead[][22] = {
    {"sidechain-lookahead0"}, {"sidechain-lookahead1"}, {"sidechain-lookahead2"}, {"sidechain-lookahead3"},
    {"sidechain-lookahead4"}, {"sidechain-lookahead5"}, {"sidechain-lookahead6"}, {"sidechain-lookahead7"}};

// LSP port tags

inline constexpr char cbe[][7] = {{"cbe_0"}, {"cbe_1"}, {"cbe_2"}, {"cbe_3"},
                                  {"cbe_4"}, {"cbe_5"}, {"cbe_6"}, {"cbe_7"}};

inline constexpr char sf[][6] = {{"sf_0"}, {"sf_1"}, {"sf_2"}, {"sf_3"}, {"sf_4"}, {"sf_5"}, {"sf_6"}, {"sf_7"}};

inline constexpr char sce[][7] = {{"sce_0"}, {"sce_1"}, {"sce_2"}, {"sce_3"},
                                  {"sce_4"}, {"sce_5"}, {"sce_6"}, {"sce_7"}};

inline constexpr char scs[][7] = {{"scs_0"}, {"scs_1"}, {"scs_2"}, {"scs_3"},
                                  {"scs_4"}, {"scs_5"}, {"scs_6"}, {"scs_7"}};

inline constexpr char scm[][7] = {{"scm_0"}, {"scm_1"}, {"scm_2"}, {"scm_3"},
                                  {"scm_4"}, {"scm_5"}, {"scm_6"}, {"scm_7"}};

inline constexpr char sla[][7] = {{"sla_0"}, {"sla_1"}, {"sla_2"}, {"sla_3"},
                                  {"sla_4"}, {"sla_5"}, {"sla_6"}, {"sla_7"}};

inline constexpr char scr[][7] = {{"scr_0"}, {"scr_1"}, {"scr_2"}, {"scr_3"},
                                  {"scr_4"}, {"scr_5"}, {"scr_6"}, {"scr_7"}};

inline constexpr char scp[][7] = {{"scp_0"}, {"scp_1"}, {"scp_2"}, {"scp_3"},
                                  {"scp_4"}, {"scp_5"}, {"scp_6"}, {"scp_7"}};

inline constexpr char sclc[][8] = {{"sclc_0"}, {"sclc_1"}, {"sclc_2"}, {"sclc_3"},
                                   {"sclc_4"}, {"sclc_5"}, {"sclc_6"}, {"sclc_7"}};

inline constexpr char schc[][8] = {{"schc_0"}, {"schc_1"}, {"schc_2"}, {"schc_3"},
                                   {"schc_4"}, {"schc_5"}, {"schc_6"}, {"schc_7"}};

inline constexpr char sclf[][8] = {{"sclf_0"}, {"sclf_1"}, {"sclf_2"}, {"sclf_3"},
                                   {"sclf_4"}, {"sclf_5"}, {"sclf_6"}, {"sclf_7"}};

inline constexpr char schf[][8] = {{"schf_0"}, {"schf_1"}, {"schf_2"}, {"schf_3"},
                                   {"schf_4"}, {"schf_5"}, {"schf_6"}, {"schf_7"}};

inline constexpr char ce[][6] = {{"ge_0"}, {"ge_1"}, {"ge_2"}, {"ge_3"}, {"ge_4"}, {"ge_5"}, {"ge_6"}, {"ge_7"}};

inline constexpr char bs[][6] = {{"bs_0"}, {"bs_1"}, {"bs_2"}, {"bs_3"}, {"bs_4"}, {"bs_5"}, {"bs_6"}, {"bs_7"}};

inline constexpr char bm[][6] = {{"bm_0"}, {"bm_1"}, {"bm_2"}, {"bm_3"}, {"bm_4"}, {"bm_5"}, {"bm_6"}, {"bm_7"}};

inline constexpr char gh[][6] = {{"gh_0"}, {"gh_1"}, {"gh_2"}, {"gh_3"}, {"gh_4"}, {"gh_5"}, {"gh_6"}, {"gh_7"}};

inline constexpr char ht[][6] = {{"ht_0"}, {"ht_1"}, {"ht_2"}, {"ht_3"}, {"ht_4"}, {"ht_5"}, {"ht_6"}, {"ht_7"}};

inline constexpr char hz[][6] = {{"hz_0"}, {"hz_1"}, {"hz_2"}, {"hz_3"}, {"hz_4"}, {"hz_5"}, {"hz_6"}, {"hz_7"}};

inline constexpr char gt[][6] = {{"gt_0"}, {"gt_1"}, {"gt_2"}, {"gt_3"}, {"gt_4"}, {"gt_5"}, {"gt_6"}, {"gt_7"}};

inline constexpr char gz[][6] = {{"gz_0"}, {"gz_1"}, {"gz_2"}, {"gz_3"}, {"gz_4"}, {"gz_5"}, {"gz_6"}, {"gz_7"}};

inline constexpr char at[][6] = {{"at_0"}, {"at_1"}, {"at_2"}, {"at_3"}, {"at_4"}, {"at_5"}, {"at_6"}, {"at_7"}};

inline constexpr char rt[][6] = {{"rt_0"}, {"rt_1"}, {"rt_2"}, {"rt_3"}, {"rt_4"}, {"rt_5"}, {"rt_6"}, {"rt_7"}};

inline constexpr char gr[][6] = {{"gr_0"}, {"gr_1"}, {"gr_2"}, {"gr_3"}, {"gr_4"}, {"gr_5"}, {"gr_6"}, {"gr_7"}};

// TODO
inline constexpr char mk[][6] = {{"mk_0"}, {"mk_1"}, {"mk_2"}, {"mk_3"}, {"mk_4"}, {"mk_5"}, {"mk_6"}, {"mk_7"}};

}  // namespace tags::multiband_gate
