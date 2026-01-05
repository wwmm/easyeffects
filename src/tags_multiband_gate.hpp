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

namespace tags::multiband_gate {

// mb gate band num const

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

constexpr auto band_gate_enable = std::to_array({{"band0GateEnable"},
                                                 {"band1GateEnable"},
                                                 {"band2GateEnable"},
                                                 {"band3GateEnable"},
                                                 {"band4GateEnable"},
                                                 {"band5GateEnable"},
                                                 {"band6GateEnable"},
                                                 std::to_array("band7GateEnable")});

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

constexpr auto band_release_time = std::to_array({{"band0ReleaseTime"},
                                                  {"band1ReleaseTime"},
                                                  {"band2ReleaseTime"},
                                                  {"band3ReleaseTime"},
                                                  {"band4ReleaseTime"},
                                                  {"band5ReleaseTime"},
                                                  {"band6ReleaseTime"},
                                                  std::to_array("band7ReleaseTime")});

constexpr auto band_hysteresis = std::to_array({{"band0Hysteresis"},
                                                {"band1Hysteresis"},
                                                {"band2Hysteresis"},
                                                {"band3Hysteresis"},
                                                {"band4Hysteresis"},
                                                {"band5Hysteresis"},
                                                {"band6Hysteresis"},
                                                std::to_array("band7Hysteresis")});

constexpr auto band_hysteresis_threshold = std::to_array({{"band0HysteresisThreshold"},
                                                          {"band1HysteresisThreshold"},
                                                          {"band2HysteresisThreshold"},
                                                          {"band3HysteresisThreshold"},
                                                          {"band4HysteresisThreshold"},
                                                          {"band5HysteresisThreshold"},
                                                          {"band6HysteresisThreshold"},
                                                          std::to_array("band7HysteresisThreshold")});

constexpr auto band_hysteresis_zone = std::to_array({{"band0HysteresisZone"},
                                                     {"band1HysteresisZone"},
                                                     {"band2HysteresisZone"},
                                                     {"band3HysteresisZone"},
                                                     {"band4HysteresisZone"},
                                                     {"band5HysteresisZone"},
                                                     {"band6HysteresisZone"},
                                                     std::to_array("band7HysteresisZone")});

constexpr auto band_curve_threshold = std::to_array({{"band0CurveThreshold"},
                                                     {"band1CurveThreshold"},
                                                     {"band2CurveThreshold"},
                                                     {"band3CurveThreshold"},
                                                     {"band4CurveThreshold"},
                                                     {"band5CurveThreshold"},
                                                     {"band6CurveThreshold"},
                                                     std::to_array("band7CurveThreshold")});

constexpr auto band_curve_zone = std::to_array({{"band0CurveZone"},
                                                {"band1CurveZone"},
                                                {"band2CurveZone"},
                                                {"band3CurveZone"},
                                                {"band4CurveZone"},
                                                {"band5CurveZone"},
                                                {"band6CurveZone"},
                                                std::to_array("band7CurveZone")});

constexpr auto band_reduction = std::to_array({{"band0Reduction"},
                                               {"band1Reduction"},
                                               {"band2Reduction"},
                                               {"band3Reduction"},
                                               {"band4Reduction"},
                                               {"band5Reduction"},
                                               {"band6Reduction"},
                                               std::to_array("band7Reduction")});

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

// LSP port tags

constexpr auto cbe = std::to_array(
    {{"cbe_0"}, {"cbe_1"}, {"cbe_2"}, {"cbe_3"}, {"cbe_4"}, {"cbe_5"}, {"cbe_6"}, std::to_array("cbe_7")});

constexpr auto sf =
    std::to_array({{"sf_0"}, {"sf_1"}, {"sf_2"}, {"sf_3"}, {"sf_4"}, {"sf_5"}, {"sf_6"}, std::to_array("sf_7")});

constexpr auto sce = std::to_array(
    {{"sce_0"}, {"sce_1"}, {"sce_2"}, {"sce_3"}, {"sce_4"}, {"sce_5"}, {"sce_6"}, std::to_array("sce_7")});

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

constexpr auto mk =
    std::to_array({{"mk_0"}, {"mk_1"}, {"mk_2"}, {"mk_3"}, {"mk_4"}, {"mk_5"}, {"mk_6"}, std::to_array("mk_7")});

}  // namespace tags::multiband_gate
