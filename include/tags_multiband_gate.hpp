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

namespace tags::multiband_gate {

// gsettings keys tags

constexpr auto band_enable = std::to_array({{"enable-band0"},
                                            {"enable-band1"},
                                            {"enable-band2"},
                                            {"enable-band3"},
                                            {"enable-band4"},
                                            {"enable-band5"},
                                            {"enable-band6"},
                                            std::to_array("enable-band7")});

constexpr auto band_gate_enable = std::to_array({{"gate-enable0"},
                                                 {"gate-enable1"},
                                                 {"gate-enable2"},
                                                 {"gate-enable3"},
                                                 {"gate-enable4"},
                                                 {"gate-enable5"},
                                                 {"gate-enable6"},
                                                 std::to_array("gate-enable7")});

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

constexpr auto band_release_time = std::to_array({{"release-time0"},
                                                  {"release-time1"},
                                                  {"release-time2"},
                                                  {"release-time3"},
                                                  {"release-time4"},
                                                  {"release-time5"},
                                                  {"release-time6"},
                                                  std::to_array("release-time7")});

constexpr auto band_hysteresis = std::to_array({{"hysteresis0"},
                                                {"hysteresis1"},
                                                {"hysteresis2"},
                                                {"hysteresis3"},
                                                {"hysteresis4"},
                                                {"hysteresis5"},
                                                {"hysteresis6"},
                                                std::to_array("hysteresis7")});

constexpr auto band_hysteresis_threshold = std::to_array({{"hysteresis-threshold0"},
                                                          {"hysteresis-threshold1"},
                                                          {"hysteresis-threshold2"},
                                                          {"hysteresis-threshold3"},
                                                          {"hysteresis-threshold4"},
                                                          {"hysteresis-threshold5"},
                                                          {"hysteresis-threshold6"},
                                                          std::to_array("hysteresis-threshold7")});

constexpr auto band_hysteresis_zone = std::to_array({{"hysteresis-zone0"},
                                                     {"hysteresis-zone1"},
                                                     {"hysteresis-zone2"},
                                                     {"hysteresis-zone3"},
                                                     {"hysteresis-zone4"},
                                                     {"hysteresis-zone5"},
                                                     {"hysteresis-zone6"},
                                                     std::to_array("hysteresis-zone7")});

constexpr auto band_curve_threshold = std::to_array({{"curve-threshold0"},
                                                     {"curve-threshold1"},
                                                     {"curve-threshold2"},
                                                     {"curve-threshold3"},
                                                     {"curve-threshold4"},
                                                     {"curve-threshold5"},
                                                     {"curve-threshold6"},
                                                     std::to_array("curve-threshold7")});

constexpr auto band_curve_zone = std::to_array({{"curve-zone0"},
                                                {"curve-zone1"},
                                                {"curve-zone2"},
                                                {"curve-zone3"},
                                                {"curve-zone4"},
                                                {"curve-zone5"},
                                                {"curve-zone6"},
                                                std::to_array("curve-zone7")});

constexpr auto band_reduction = std::to_array({{"reduction0"},
                                               {"reduction1"},
                                               {"reduction2"},
                                               {"reduction3"},
                                               {"reduction4"},
                                               {"reduction5"},
                                               {"reduction6"},
                                               std::to_array("reduction7")});

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

// LSP port tags

constexpr auto cbe = std::to_array(
    {{"cbe_0"}, {"cbe_1"}, {"cbe_2"}, {"cbe_3"}, {"cbe_4"}, {"cbe_5"}, {"cbe_6"}, std::to_array("cbe_7")});

constexpr auto sf =
    std::to_array({{"sf_0"}, {"sf_1"}, {"sf_2"}, {"sf_3"}, {"sf_4"}, {"sf_5"}, {"sf_6"}, std::to_array("sf_7")});

constexpr auto sce = std::to_array(
    {{"sce_0"}, {"sce_1"}, {"sce_2"}, {"sce_3"}, {"sce_4"}, {"sce_5"}, {"sce_6"}, std::to_array("sce_7")});

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

constexpr auto ce =
    std::to_array({{"ge_0"}, {"ge_1"}, {"ge_2"}, {"ge_3"}, {"ge_4"}, {"ge_5"}, {"ge_6"}, std::to_array("ge_7")});

constexpr auto bs =
    std::to_array({{"bs_0"}, {"bs_1"}, {"bs_2"}, {"bs_3"}, {"bs_4"}, {"bs_5"}, {"bs_6"}, std::to_array("bs_7")});

constexpr auto bm =
    std::to_array({{"bm_0"}, {"bm_1"}, {"bm_2"}, {"bm_3"}, {"bm_4"}, {"bm_5"}, {"bm_6"}, std::to_array("bm_7")});

constexpr auto gh =
    std::to_array({{"gh_0"}, {"gh_1"}, {"gh_2"}, {"gh_3"}, {"gh_4"}, {"gh_5"}, {"gh_6"}, std::to_array("gh_7")});

constexpr auto ht =
    std::to_array({{"ht_0"}, {"ht_1"}, {"ht_2"}, {"ht_3"}, {"ht_4"}, {"ht_5"}, {"ht_6"}, std::to_array("ht_7")});

constexpr auto hz =
    std::to_array({{"hz_0"}, {"hz_1"}, {"hz_2"}, {"hz_3"}, {"hz_4"}, {"hz_5"}, {"hz_6"}, std::to_array("hz_7")});

constexpr auto gt =
    std::to_array({{"gt_0"}, {"gt_1"}, {"gt_2"}, {"gt_3"}, {"gt_4"}, {"gt_5"}, {"gt_6"}, std::to_array("gt_7")});

constexpr auto gz =
    std::to_array({{"gz_0"}, {"gz_1"}, {"gz_2"}, {"gz_3"}, {"gz_4"}, {"gz_5"}, {"gz_6"}, std::to_array("gz_7")});

constexpr auto at =
    std::to_array({{"at_0"}, {"at_1"}, {"at_2"}, {"at_3"}, {"at_4"}, {"at_5"}, {"at_6"}, std::to_array("at_7")});

constexpr auto rt =
    std::to_array({{"rt_0"}, {"rt_1"}, {"rt_2"}, {"rt_3"}, {"rt_4"}, {"rt_5"}, {"rt_6"}, std::to_array("rt_7")});

constexpr auto gr =
    std::to_array({{"gr_0"}, {"gr_1"}, {"gr_2"}, {"gr_3"}, {"gr_4"}, {"gr_5"}, {"gr_6"}, std::to_array("gr_7")});

// TODO
constexpr auto mk =
    std::to_array({{"mk_0"}, {"mk_1"}, {"mk_2"}, {"mk_3"}, {"mk_4"}, {"mk_5"}, {"mk_6"}, std::to_array("mk_7")});

}  // namespace tags::multiband_gate
