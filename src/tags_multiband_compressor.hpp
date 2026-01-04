/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>

namespace tags::multiband_compressor {

// mb comp band num const

constexpr unsigned n_bands = 8U;

// band tags

constexpr auto band_enable = std::to_array({{"band0Enable"},
                                            {"band1Enable"},
                                            {"band2Enable"},
                                            {"band3Enable"},
                                            {"band4Enable"},
                                            {"band5Enable"},
                                            {"band6Enable"},
                                            std::to_array("band7Enable")});

constexpr auto band_compression_mode = std::to_array({{"band0CompressionMode"},
                                                      {"band1CompressionMode"},
                                                      {"band2CompressionMode"},
                                                      {"band3CompressionMode"},
                                                      {"band4CompressionMode"},
                                                      {"band5CompressionMode"},
                                                      {"band6CompressionMode"},
                                                      std::to_array("band7CompressionMode")});

constexpr auto band_compressor_enable = std::to_array({{"band0CompressorEnable"},
                                                       {"band1CompressorEnable"},
                                                       {"band2CompressorEnable"},
                                                       {"band3CompressorEnable"},
                                                       {"band4CompressorEnable"},
                                                       {"band5CompressorEnable"},
                                                       {"band6CompressorEnable"},
                                                       std::to_array("band7CompressorEnable")});

constexpr auto band_split_frequency = std::to_array({{"band0SplitFrequency"},
                                                     {"band1SplitFrequency"},
                                                     {"band2SplitFrequency"},
                                                     {"band3SplitFrequency"},
                                                     {"band4SplitFrequency"},
                                                     {"band5SplitFrequency"},
                                                     {"band6SplitFrequency"},
                                                     std::to_array("band7SplitFrequency")});

constexpr auto band_mute = std::to_array({{"band0Mute"},
                                          {"band1Mute"},
                                          {"band2Mute"},
                                          {"band3Mute"},
                                          {"band4Mute"},
                                          {"band5Mute"},
                                          {"band6Mute"},
                                          std::to_array("band7Mute")});

constexpr auto band_solo = std::to_array({{"band0Solo"},
                                          {"band1Solo"},
                                          {"band2Solo"},
                                          {"band3Solo"},
                                          {"band4Solo"},
                                          {"band5Solo"},
                                          {"band6Solo"},
                                          std::to_array("band7Solo")});

constexpr auto band_lowcut_filter = std::to_array({{"band0SidechainCustomLowcutFilter"},
                                                   {"band1SidechainCustomLowcutFilter"},
                                                   {"band2SidechainCustomLowcutFilter"},
                                                   {"band3SidechainCustomLowcutFilter"},
                                                   {"band4SidechainCustomLowcutFilter"},
                                                   {"band5SidechainCustomLowcutFilter"},
                                                   {"band6SidechainCustomLowcutFilter"},
                                                   std::to_array("band7SidechainCustomLowcutFilter")});

constexpr auto band_highcut_filter = std::to_array({{"band0SidechainCustomHighcutFilter"},
                                                    {"band1SidechainCustomHighcutFilter"},
                                                    {"band2SidechainCustomHighcutFilter"},
                                                    {"band3SidechainCustomHighcutFilter"},
                                                    {"band4SidechainCustomHighcutFilter"},
                                                    {"band5SidechainCustomHighcutFilter"},
                                                    {"band6SidechainCustomHighcutFilter"},
                                                    std::to_array("band7SidechainCustomHighcutFilter")});

constexpr auto band_sidechain_type = std::to_array({{"band0SidechainType"},
                                                    {"band1SidechainType"},
                                                    {"band2SidechainType"},
                                                    {"band3SidechainType"},
                                                    {"band4SidechainType"},
                                                    {"band5SidechainType"},
                                                    {"band6SidechainType"},
                                                    std::to_array("band7SidechainType")});

constexpr auto band_sidechain_mode = std::to_array({{"band0SidechainMode"},
                                                    {"band1SidechainMode"},
                                                    {"band2SidechainMode"},
                                                    {"band3SidechainMode"},
                                                    {"band4SidechainMode"},
                                                    {"band5SidechainMode"},
                                                    {"band6SidechainMode"},
                                                    std::to_array("band7SidechainMode")});

constexpr auto band_sidechain_source = std::to_array({{"band0SidechainSource"},
                                                      {"band1SidechainSource"},
                                                      {"band2SidechainSource"},
                                                      {"band3SidechainSource"},
                                                      {"band4SidechainSource"},
                                                      {"band5SidechainSource"},
                                                      {"band6SidechainSource"},
                                                      std::to_array("band7SidechainSource")});

constexpr auto band_stereo_split_source = std::to_array({{"band0StereoSplitSource"},
                                                         {"band1StereoSplitSource"},
                                                         {"band2StereoSplitSource"},
                                                         {"band3StereoSplitSource"},
                                                         {"band4StereoSplitSource"},
                                                         {"band5StereoSplitSource"},
                                                         {"band6StereoSplitSource"},
                                                         std::to_array("band7StereoSplitSource")});

constexpr auto band_lowcut_filter_frequency = std::to_array({{"band0SidechainLowcutFrequency"},
                                                             {"band1SidechainLowcutFrequency"},
                                                             {"band2SidechainLowcutFrequency"},
                                                             {"band3SidechainLowcutFrequency"},
                                                             {"band4SidechainLowcutFrequency"},
                                                             {"band5SidechainLowcutFrequency"},
                                                             {"band6SidechainLowcutFrequency"},
                                                             std::to_array("band7SidechainLowcutFrequency")});

constexpr auto band_highcut_filter_frequency = std::to_array({{"band0SidechainHighcutFrequency"},
                                                              {"band1SidechainHighcutFrequency"},
                                                              {"band2SidechainHighcutFrequency"},
                                                              {"band3SidechainHighcutFrequency"},
                                                              {"band4SidechainHighcutFrequency"},
                                                              {"band5SidechainHighcutFrequency"},
                                                              {"band6SidechainHighcutFrequency"},
                                                              std::to_array("band7SidechainHighcutFrequency")});

constexpr auto band_attack_time = std::to_array({{"band0AttackTime"},
                                                 {"band1AttackTime"},
                                                 {"band2AttackTime"},
                                                 {"band3AttackTime"},
                                                 {"band4AttackTime"},
                                                 {"band5AttackTime"},
                                                 {"band6AttackTime"},
                                                 std::to_array("band7AttackTime")});

inline constexpr auto band_attack_threshold = std::to_array({{"band0AttackThreshold"},
                                                             {"band1AttackThreshold"},
                                                             {"band2AttackThreshold"},
                                                             {"band3AttackThreshold"},
                                                             {"band4AttackThreshold"},
                                                             {"band5AttackThreshold"},
                                                             {"band6AttackThreshold"},
                                                             std::to_array("band7AttackThreshold")});

inline constexpr auto band_release_time = std::to_array({{"band0ReleaseTime"},
                                                         {"band1ReleaseTime"},
                                                         {"band2ReleaseTime"},
                                                         {"band3ReleaseTime"},
                                                         {"band4ReleaseTime"},
                                                         {"band5ReleaseTime"},
                                                         {"band6ReleaseTime"},
                                                         std::to_array("band7ReleaseTime")});

inline constexpr auto band_release_threshold = std::to_array({{"band0ReleaseThreshold"},
                                                              {"band1ReleaseThreshold"},
                                                              {"band2ReleaseThreshold"},
                                                              {"band3ReleaseThreshold"},
                                                              {"band4ReleaseThreshold"},
                                                              {"band5ReleaseThreshold"},
                                                              {"band6ReleaseThreshold"},
                                                              std::to_array("band7ReleaseThreshold")});

constexpr auto band_ratio = std::to_array({{"band0Ratio"},
                                           {"band1Ratio"},
                                           {"band2Ratio"},
                                           {"band3Ratio"},
                                           {"band4Ratio"},
                                           {"band5Ratio"},
                                           {"band6Ratio"},
                                           std::to_array("band7Ratio")});

constexpr auto band_knee = std::to_array({{"band0Knee"},
                                          {"band1Knee"},
                                          {"band2Knee"},
                                          {"band3Knee"},
                                          {"band4Knee"},
                                          {"band5Knee"},
                                          {"band6Knee"},
                                          std::to_array("band7Knee")});

constexpr auto band_makeup = std::to_array({{"band0Makeup"},
                                            {"band1Makeup"},
                                            {"band2Makeup"},
                                            {"band3Makeup"},
                                            {"band4Makeup"},
                                            {"band5Makeup"},
                                            {"band6Makeup"},
                                            std::to_array("band7Makeup")});

constexpr auto band_sidechain_preamp = std::to_array({{"band0SidechainPreamp"},
                                                      {"band1SidechainPreamp"},
                                                      {"band2SidechainPreamp"},
                                                      {"band3SidechainPreamp"},
                                                      {"band4SidechainPreamp"},
                                                      {"band5SidechainPreamp"},
                                                      {"band6SidechainPreamp"},
                                                      std::to_array("band7SidechainPreamp")});

constexpr auto band_sidechain_reactivity = std::to_array({{"band0SidechainReactivity"},
                                                          {"band1SidechainReactivity"},
                                                          {"band2SidechainReactivity"},
                                                          {"band3SidechainReactivity"},
                                                          {"band4SidechainReactivity"},
                                                          {"band5SidechainReactivity"},
                                                          {"band6SidechainReactivity"},
                                                          std::to_array("band7SidechainReactivity")});

constexpr auto band_sidechain_lookahead = std::to_array({{"band0SidechainLookahead"},
                                                         {"band1SidechainLookahead"},
                                                         {"band2SidechainLookahead"},
                                                         {"band3SidechainLookahead"},
                                                         {"band4SidechainLookahead"},
                                                         {"band5SidechainLookahead"},
                                                         {"band6SidechainLookahead"},
                                                         std::to_array("band7SidechainLookahead")});

constexpr auto band_boost_amount = std::to_array({{"band0BoostAmount"},
                                                  {"band1BoostAmount"},
                                                  {"band2BoostAmount"},
                                                  {"band3BoostAmount"},
                                                  {"band4BoostAmount"},
                                                  {"band5BoostAmount"},
                                                  {"band6BoostAmount"},
                                                  std::to_array("band7BoostAmount")});

constexpr auto band_boost_threshold = std::to_array({{"band0BoostThreshold"},
                                                     {"band1BoostThreshold"},
                                                     {"band2BoostThreshold"},
                                                     {"band3BoostThreshold"},
                                                     {"band4BoostThreshold"},
                                                     {"band5BoostThreshold"},
                                                     {"band6BoostThreshold"},
                                                     std::to_array("band7BoostThreshold")});

// LSP port tags

constexpr auto sce = std::to_array(
    {{"sce_0"}, {"sce_1"}, {"sce_2"}, {"sce_3"}, {"sce_4"}, {"sce_5"}, {"sce_6"}, std::to_array("sce_7")});

constexpr auto cbe = std::to_array(
    {{"cbe_0"}, {"cbe_1"}, {"cbe_2"}, {"cbe_3"}, {"cbe_4"}, {"cbe_5"}, {"cbe_6"}, std::to_array("cbe_7")});

constexpr auto sf =
    std::to_array({{"sf_0"}, {"sf_1"}, {"sf_2"}, {"sf_3"}, {"sf_4"}, {"sf_5"}, {"sf_6"}, std::to_array("sf_7")});

constexpr auto sscs = std::to_array(
    {{"sscs_0"}, {"sscs_1"}, {"sscs_2"}, {"sscs_3"}, {"sscs_4"}, {"sscs_5"}, {"sscs_6"}, std::to_array("sscs_7")});

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
