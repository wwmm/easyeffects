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

#include <array>

namespace tags::multiband_compressor {

// gsettings keys tags

constexpr auto band_enable = std::to_array({{"enable-band0"},
                                            {"enable-band1"},
                                            {"enable-band2"},
                                            {"enable-band3"},
                                            {"enable-band4"},
                                            {"enable-band5"},
                                            {"enable-band6"},
                                            std::to_array("enable-band7")});

constexpr auto band_compression_mode = std::to_array({{"compression-mode0"},
                                                      {"compression-mode1"},
                                                      {"compression-mode2"},
                                                      {"compression-mode3"},
                                                      {"compression-mode4"},
                                                      {"compression-mode5"},
                                                      {"compression-mode6"},
                                                      std::to_array("compression-mode7")});

constexpr auto band_compressor_enable = std::to_array({{"compressor-enable0"},
                                                       {"compressor-enable1"},
                                                       {"compressor-enable2"},
                                                       {"compressor-enable3"},
                                                       {"compressor-enable4"},
                                                       {"compressor-enable5"},
                                                       {"compressor-enable6"},
                                                       std::to_array("compressor-enable7")});

constexpr auto band_split_frequency = std::to_array({{"split-frequency0"},
                                                     {"split-frequency1"},
                                                     {"split-frequency2"},
                                                     {"split-frequency3"},
                                                     {"split-frequency4"},
                                                     {"split-frequency5"},
                                                     {"split-frequency6"},
                                                     std::to_array("split-frequency7")});

constexpr auto band_mute = std::to_array(
    {{"mute0"}, {"mute1"}, {"mute2"}, {"mute3"}, {"mute4"}, {"mute5"}, {"mute6"}, std::to_array("mute7")});

constexpr auto band_solo = std::to_array(
    {{"solo0"}, {"solo1"}, {"solo2"}, {"solo3"}, {"solo4"}, {"solo5"}, {"solo6"}, std::to_array("solo7")});

constexpr auto band_lowcut_filter = std::to_array({{"sidechain-custom-lowcut-filter0"},
                                                   {"sidechain-custom-lowcut-filter1"},
                                                   {"sidechain-custom-lowcut-filter2"},
                                                   {"sidechain-custom-lowcut-filter3"},
                                                   {"sidechain-custom-lowcut-filter4"},
                                                   {"sidechain-custom-lowcut-filter5"},
                                                   {"sidechain-custom-lowcut-filter6"},
                                                   std::to_array("sidechain-custom-lowcut-filter7")});

constexpr auto band_highcut_filter = std::to_array({{"sidechain-custom-highcut-filter0"},
                                                    {"sidechain-custom-highcut-filter1"},
                                                    {"sidechain-custom-highcut-filter2"},
                                                    {"sidechain-custom-highcut-filter3"},
                                                    {"sidechain-custom-highcut-filter4"},
                                                    {"sidechain-custom-highcut-filter5"},
                                                    {"sidechain-custom-highcut-filter6"},
                                                    std::to_array("sidechain-custom-highcut-filter7")});

constexpr auto band_external_sidechain = std::to_array({{"external-sidechain0"},
                                                        {"external-sidechain1"},
                                                        {"external-sidechain2"},
                                                        {"external-sidechain3"},
                                                        {"external-sidechain4"},
                                                        {"external-sidechain5"},
                                                        {"external-sidechain6"},
                                                        std::to_array("external-sidechain7")});

constexpr auto band_sidechain_mode = std::to_array({{"sidechain-mode0"},
                                                    {"sidechain-mode1"},
                                                    {"sidechain-mode2"},
                                                    {"sidechain-mode3"},
                                                    {"sidechain-mode4"},
                                                    {"sidechain-mode5"},
                                                    {"sidechain-mode6"},
                                                    std::to_array("sidechain-mode7")});

constexpr auto band_sidechain_source = std::to_array({{"sidechain-source0"},
                                                      {"sidechain-source1"},
                                                      {"sidechain-source2"},
                                                      {"sidechain-source3"},
                                                      {"sidechain-source4"},
                                                      {"sidechain-source5"},
                                                      {"sidechain-source6"},
                                                      std::to_array("sidechain-source7")});

constexpr auto band_lowcut_filter_frequency = std::to_array({{"sidechain-lowcut-frequency0"},
                                                             {"sidechain-lowcut-frequency1"},
                                                             {"sidechain-lowcut-frequency2"},
                                                             {"sidechain-lowcut-frequency3"},
                                                             {"sidechain-lowcut-frequency4"},
                                                             {"sidechain-lowcut-frequency5"},
                                                             {"sidechain-lowcut-frequency6"},
                                                             std::to_array("sidechain-lowcut-frequency7")});

constexpr auto band_highcut_filter_frequency = std::to_array({{"sidechain-highcut-frequency0"},
                                                              {"sidechain-highcut-frequency1"},
                                                              {"sidechain-highcut-frequency2"},
                                                              {"sidechain-highcut-frequency3"},
                                                              {"sidechain-highcut-frequency4"},
                                                              {"sidechain-highcut-frequency5"},
                                                              {"sidechain-highcut-frequency6"},
                                                              std::to_array("sidechain-highcut-frequency7")});

constexpr auto band_attack_time = std::to_array({{"attack-time0"},
                                                 {"attack-time1"},
                                                 {"attack-time2"},
                                                 {"attack-time3"},
                                                 {"attack-time4"},
                                                 {"attack-time5"},
                                                 {"attack-time6"},
                                                 std::to_array("attack-time7")});

inline constexpr auto band_attack_threshold = std::to_array({{"attack-threshold0"},
                                                             {"attack-threshold1"},
                                                             {"attack-threshold2"},
                                                             {"attack-threshold3"},
                                                             {"attack-threshold4"},
                                                             {"attack-threshold5"},
                                                             {"attack-threshold6"},
                                                             std::to_array("attack-threshold7")});

inline constexpr auto band_release_time = std::to_array({{"release-time0"},
                                                         {"release-time1"},
                                                         {"release-time2"},
                                                         {"release-time3"},
                                                         {"release-time4"},
                                                         {"release-time5"},
                                                         {"release-time6"},
                                                         std::to_array("release-time7")});

inline constexpr auto band_release_threshold = std::to_array({{"release-threshold0"},
                                                              {"release-threshold1"},
                                                              {"release-threshold2"},
                                                              {"release-threshold3"},
                                                              {"release-threshold4"},
                                                              {"release-threshold5"},
                                                              {"release-threshold6"},
                                                              std::to_array("release-threshold7")});

constexpr auto band_ratio = std::to_array(
    {{"ratio0"}, {"ratio1"}, {"ratio2"}, {"ratio3"}, {"ratio4"}, {"ratio5"}, {"ratio6"}, std::to_array("ratio7")});

constexpr auto band_knee = std::to_array(
    {{"knee0"}, {"knee1"}, {"knee2"}, {"knee3"}, {"knee4"}, {"knee5"}, {"knee6"}, std::to_array("knee7")});

constexpr auto band_makeup = std::to_array({{"makeup0"},
                                            {"makeup1"},
                                            {"makeup2"},
                                            {"makeup3"},
                                            {"makeup4"},
                                            {"makeup5"},
                                            {"makeup6"},
                                            std::to_array("makeup7")});

constexpr auto band_sidechain_preamp = std::to_array({{"sidechain-preamp0"},
                                                      {"sidechain-preamp1"},
                                                      {"sidechain-preamp2"},
                                                      {"sidechain-preamp3"},
                                                      {"sidechain-preamp4"},
                                                      {"sidechain-preamp5"},
                                                      {"sidechain-preamp6"},
                                                      std::to_array("sidechain-preamp7")});

constexpr auto band_sidechain_reactivity = std::to_array({{"sidechain-reactivity0"},
                                                          {"sidechain-reactivity1"},
                                                          {"sidechain-reactivity2"},
                                                          {"sidechain-reactivity3"},
                                                          {"sidechain-reactivity4"},
                                                          {"sidechain-reactivity5"},
                                                          {"sidechain-reactivity6"},
                                                          std::to_array("sidechain-reactivity7")});

constexpr auto band_sidechain_lookahead = std::to_array({{"sidechain-lookahead0"},
                                                         {"sidechain-lookahead1"},
                                                         {"sidechain-lookahead2"},
                                                         {"sidechain-lookahead3"},
                                                         {"sidechain-lookahead4"},
                                                         {"sidechain-lookahead5"},
                                                         {"sidechain-lookahead6"},
                                                         std::to_array("sidechain-lookahead7")});

constexpr auto band_boost_amount = std::to_array({{"boost-amount0"},
                                                  {"boost-amount1"},
                                                  {"boost-amount2"},
                                                  {"boost-amount3"},
                                                  {"boost-amount4"},
                                                  {"boost-amount5"},
                                                  {"boost-amount6"},
                                                  std::to_array("boost-amount7")});

constexpr auto band_boost_threshold = std::to_array({{"boost-threshold0"},
                                                     {"boost-threshold1"},
                                                     {"boost-threshold2"},
                                                     {"boost-threshold3"},
                                                     {"boost-threshold4"},
                                                     {"boost-threshold5"},
                                                     {"boost-threshold6"},
                                                     std::to_array("boost-threshold7")});

// LSP port tags

constexpr auto sce = std::to_array(
    {{"sce_0"}, {"sce_1"}, {"sce_2"}, {"sce_3"}, {"sce_4"}, {"sce_5"}, {"sce_6"}, std::to_array("sce_7")});

constexpr auto cbe = std::to_array(
    {{"cbe_0"}, {"cbe_1"}, {"cbe_2"}, {"cbe_3"}, {"cbe_4"}, {"cbe_5"}, {"cbe_6"}, std::to_array("cbe_7")});

constexpr auto sf =
    std::to_array({{"sf_0"}, {"sf_1"}, {"sf_2"}, {"sf_3"}, {"sf_4"}, {"sf_5"}, {"sf_6"}, std::to_array("sf_7")});

constexpr auto scs = std::to_array(
    {{"scs_0"}, {"scs_1"}, {"scs_2"}, {"scs_3"}, {"scs_4"}, {"scs_5"}, {"scs_6"}, std::to_array("scs_7")});

constexpr auto scm = std::to_array(
    {{"scm_0"}, {"scm_1"}, {"scm_2"}, {"scm_3"}, {"scm_4"}, {"scm_5"}, {"scm_6"}, std::to_array("scm_7")});

constexpr auto sla = std::to_array(
    {{"sla_0"}, {"sla_1"}, {"sla_2"}, {"sla_3"}, {"sla_4"}, {"sla_5"}, {"sla_6"}, std::to_array("sla_7")});

constexpr auto scr = std::to_array(
    {{"scr_0"}, {"scr_1"}, {"scr_2"}, {"scr_3"}, {"scr_4"}, {"scr_5"}, {"scr_6"}, std::to_array("scr_7")});

constexpr auto scp = std::to_array(
    {{"scp_0"}, {"scp_1"}, {"scp_2"}, {"scp_3"}, {"scp_4"}, {"scp_5"}, {"scp_6"}, std::to_array("scp_7")});

constexpr auto sclc = std::to_array(
    {{"sclc_0"}, {"sclc_1"}, {"sclc_2"}, {"sclc_3"}, {"sclc_4"}, {"sclc_5"}, {"sclc_6"}, std::to_array("sclc_7")});

constexpr auto schc = std::to_array(
    {{"schc_0"}, {"schc_1"}, {"schc_2"}, {"schc_3"}, {"schc_4"}, {"schc_5"}, {"schc_6"}, std::to_array("schc_7")});

constexpr auto sclf = std::to_array(
    {{"sclf_0"}, {"sclf_1"}, {"sclf_2"}, {"sclf_3"}, {"sclf_4"}, {"sclf_5"}, {"sclf_6"}, std::to_array("sclf_7")});

constexpr auto schf = std::to_array(
    {{"schf_0"}, {"schf_1"}, {"schf_2"}, {"schf_3"}, {"schf_4"}, {"schf_5"}, {"schf_6"}, std::to_array("schf_7")});

constexpr auto cm =
    std::to_array({{"cm_0"}, {"cm_1"}, {"cm_2"}, {"cm_3"}, {"cm_4"}, {"cm_5"}, {"cm_6"}, std::to_array("cm_7")});

constexpr auto ce =
    std::to_array({{"ce_0"}, {"ce_1"}, {"ce_2"}, {"ce_3"}, {"ce_4"}, {"ce_5"}, {"ce_6"}, std::to_array("ce_7")});

constexpr auto bs =
    std::to_array({{"bs_0"}, {"bs_1"}, {"bs_2"}, {"bs_3"}, {"bs_4"}, {"bs_5"}, {"bs_6"}, std::to_array("bs_7")});

constexpr auto bm =
    std::to_array({{"bm_0"}, {"bm_1"}, {"bm_2"}, {"bm_3"}, {"bm_4"}, {"bm_5"}, {"bm_6"}, std::to_array("bm_7")});

constexpr auto al =
    std::to_array({{"al_0"}, {"al_1"}, {"al_2"}, {"al_3"}, {"al_4"}, {"al_5"}, {"al_6"}, std::to_array("al_7")});

constexpr auto at =
    std::to_array({{"at_0"}, {"at_1"}, {"at_2"}, {"at_3"}, {"at_4"}, {"at_5"}, {"at_6"}, std::to_array("at_7")});

constexpr auto rrl = std::to_array(
    {{"rrl_0"}, {"rrl_1"}, {"rrl_2"}, {"rrl_3"}, {"rrl_4"}, {"rrl_5"}, {"rrl_6"}, std::to_array("rrl_7")});

constexpr auto rt =
    std::to_array({{"rt_0"}, {"rt_1"}, {"rt_2"}, {"rt_3"}, {"rt_4"}, {"rt_5"}, {"rt_6"}, std::to_array("rt_7")});

constexpr auto cr =
    std::to_array({{"cr_0"}, {"cr_1"}, {"cr_2"}, {"cr_3"}, {"cr_4"}, {"cr_5"}, {"cr_6"}, std::to_array("cr_7")});

constexpr auto kn =
    std::to_array({{"kn_0"}, {"kn_1"}, {"kn_2"}, {"kn_3"}, {"kn_4"}, {"kn_5"}, {"kn_6"}, std::to_array("kn_7")});

constexpr auto bth = std::to_array(
    {{"bth_0"}, {"bth_1"}, {"bth_2"}, {"bth_3"}, {"bth_4"}, {"bth_5"}, {"bth_6"}, std::to_array("bth_7")});

constexpr auto bsa = std::to_array(
    {{"bsa_0"}, {"bsa_1"}, {"bsa_2"}, {"bsa_3"}, {"bsa_4"}, {"bsa_5"}, {"bsa_6"}, std::to_array("bsa_7")});

constexpr auto mk =
    std::to_array({{"mk_0"}, {"mk_1"}, {"mk_2"}, {"mk_3"}, {"mk_4"}, {"mk_5"}, {"mk_6"}, std::to_array("mk_7")});

}  // namespace tags::multiband_compressor