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

namespace tags::equalizer {

// bands

constexpr auto band_id = std::to_array(
    {"band0",  "band1",  "band2",  "band3",  "band4",  "band5",  "band6",  "band7",  "band8",  "band9",  "band10",
     "band11", "band12", "band13", "band14", "band15", "band16", "band17", "band18", "band19", "band20", "band21",
     "band22", "band23", "band24", "band25", "band26", "band27", "band28", "band29", "band30", "band31"});

constexpr auto band_type =
    std::to_array({{"band0-type"},  {"band1-type"},  {"band2-type"},  {"band3-type"},
                   {"band4-type"},  {"band5-type"},  {"band6-type"},  {"band7-type"},
                   {"band8-type"},  {"band9-type"},  {"band10-type"}, {"band11-type"},
                   {"band12-type"}, {"band13-type"}, {"band14-type"}, {"band15-type"},
                   {"band16-type"}, {"band17-type"}, {"band18-type"}, {"band19-type"},
                   {"band20-type"}, {"band21-type"}, {"band22-type"}, {"band23-type"},
                   {"band24-type"}, {"band25-type"}, {"band26-type"}, {"band27-type"},
                   {"band28-type"}, {"band29-type"}, {"band30-type"}, std::to_array("band31-type")});

constexpr auto band_mode =
    std::to_array({{"band0-mode"},  {"band1-mode"},  {"band2-mode"},  {"band3-mode"},
                   {"band4-mode"},  {"band5-mode"},  {"band6-mode"},  {"band7-mode"},
                   {"band8-mode"},  {"band9-mode"},  {"band10-mode"}, {"band11-mode"},
                   {"band12-mode"}, {"band13-mode"}, {"band14-mode"}, {"band15-mode"},
                   {"band16-mode"}, {"band17-mode"}, {"band18-mode"}, {"band19-mode"},
                   {"band20-mode"}, {"band21-mode"}, {"band22-mode"}, {"band23-mode"},
                   {"band24-mode"}, {"band25-mode"}, {"band26-mode"}, {"band27-mode"},
                   {"band28-mode"}, {"band29-mode"}, {"band30-mode"}, std::to_array("band31-mode")});

constexpr auto band_slope =
    std::to_array({{"band0-slope"},  {"band1-slope"},  {"band2-slope"},  {"band3-slope"},
                   {"band4-slope"},  {"band5-slope"},  {"band6-slope"},  {"band7-slope"},
                   {"band8-slope"},  {"band9-slope"},  {"band10-slope"}, {"band11-slope"},
                   {"band12-slope"}, {"band13-slope"}, {"band14-slope"}, {"band15-slope"},
                   {"band16-slope"}, {"band17-slope"}, {"band18-slope"}, {"band19-slope"},
                   {"band20-slope"}, {"band21-slope"}, {"band22-slope"}, {"band23-slope"},
                   {"band24-slope"}, {"band25-slope"}, {"band26-slope"}, {"band27-slope"},
                   {"band28-slope"}, {"band29-slope"}, {"band30-slope"}, std::to_array("band31-slope")});

constexpr auto band_solo =
    std::to_array({{"band0-solo"},  {"band1-solo"},  {"band2-solo"},  {"band3-solo"},
                   {"band4-solo"},  {"band5-solo"},  {"band6-solo"},  {"band7-solo"},
                   {"band8-solo"},  {"band9-solo"},  {"band10-solo"}, {"band11-solo"},
                   {"band12-solo"}, {"band13-solo"}, {"band14-solo"}, {"band15-solo"},
                   {"band16-solo"}, {"band17-solo"}, {"band18-solo"}, {"band19-solo"},
                   {"band20-solo"}, {"band21-solo"}, {"band22-solo"}, {"band23-solo"},
                   {"band24-solo"}, {"band25-solo"}, {"band26-solo"}, {"band27-solo"},
                   {"band28-solo"}, {"band29-solo"}, {"band30-solo"}, std::to_array("band31-solo")});

constexpr auto band_mute =
    std::to_array({{"band0-mute"},  {"band1-mute"},  {"band2-mute"},  {"band3-mute"},
                   {"band4-mute"},  {"band5-mute"},  {"band6-mute"},  {"band7-mute"},
                   {"band8-mute"},  {"band9-mute"},  {"band10-mute"}, {"band11-mute"},
                   {"band12-mute"}, {"band13-mute"}, {"band14-mute"}, {"band15-mute"},
                   {"band16-mute"}, {"band17-mute"}, {"band18-mute"}, {"band19-mute"},
                   {"band20-mute"}, {"band21-mute"}, {"band22-mute"}, {"band23-mute"},
                   {"band24-mute"}, {"band25-mute"}, {"band26-mute"}, {"band27-mute"},
                   {"band28-mute"}, {"band29-mute"}, {"band30-mute"}, std::to_array("band31-mute")});

constexpr auto band_frequency = std::to_array(
    {{"band0-frequency"},  {"band1-frequency"},  {"band2-frequency"},  {"band3-frequency"},
     {"band4-frequency"},  {"band5-frequency"},  {"band6-frequency"},  {"band7-frequency"},
     {"band8-frequency"},  {"band9-frequency"},  {"band10-frequency"}, {"band11-frequency"},
     {"band12-frequency"}, {"band13-frequency"}, {"band14-frequency"}, {"band15-frequency"},
     {"band16-frequency"}, {"band17-frequency"}, {"band18-frequency"}, {"band19-frequency"},
     {"band20-frequency"}, {"band21-frequency"}, {"band22-frequency"}, {"band23-frequency"},
     {"band24-frequency"}, {"band25-frequency"}, {"band26-frequency"}, {"band27-frequency"},
     {"band28-frequency"}, {"band29-frequency"}, {"band30-frequency"}, std::to_array("band31-frequency")});

constexpr auto band_q = std::to_array({{"band0-q"},  {"band1-q"},  {"band2-q"},  {"band3-q"},
                                       {"band4-q"},  {"band5-q"},  {"band6-q"},  {"band7-q"},
                                       {"band8-q"},  {"band9-q"},  {"band10-q"}, {"band11-q"},
                                       {"band12-q"}, {"band13-q"}, {"band14-q"}, {"band15-q"},
                                       {"band16-q"}, {"band17-q"}, {"band18-q"}, {"band19-q"},
                                       {"band20-q"}, {"band21-q"}, {"band22-q"}, {"band23-q"},
                                       {"band24-q"}, {"band25-q"}, {"band26-q"}, {"band27-q"},
                                       {"band28-q"}, {"band29-q"}, {"band30-q"}, std::to_array("band31-q")});

constexpr auto band_gain =
    std::to_array({{"band0-gain"},  {"band1-gain"},  {"band2-gain"},  {"band3-gain"},
                   {"band4-gain"},  {"band5-gain"},  {"band6-gain"},  {"band7-gain"},
                   {"band8-gain"},  {"band9-gain"},  {"band10-gain"}, {"band11-gain"},
                   {"band12-gain"}, {"band13-gain"}, {"band14-gain"}, {"band15-gain"},
                   {"band16-gain"}, {"band17-gain"}, {"band18-gain"}, {"band19-gain"},
                   {"band20-gain"}, {"band21-gain"}, {"band22-gain"}, {"band23-gain"},
                   {"band24-gain"}, {"band25-gain"}, {"band26-gain"}, {"band27-gain"},
                   {"band28-gain"}, {"band29-gain"}, {"band30-gain"}, std::to_array("band31-gain")});

// left channel

constexpr auto ftl = std::to_array(
    {{"ftl_0"},  {"ftl_1"},  {"ftl_2"},  {"ftl_3"},  {"ftl_4"},  {"ftl_5"},  {"ftl_6"},  {"ftl_7"},
     {"ftl_8"},  {"ftl_9"},  {"ftl_10"}, {"ftl_11"}, {"ftl_12"}, {"ftl_13"}, {"ftl_14"}, {"ftl_15"},
     {"ftl_16"}, {"ftl_17"}, {"ftl_18"}, {"ftl_19"}, {"ftl_20"}, {"ftl_21"}, {"ftl_22"}, {"ftl_23"},
     {"ftl_24"}, {"ftl_25"}, {"ftl_26"}, {"ftl_27"}, {"ftl_28"}, {"ftl_29"}, {"ftl_30"}, std::to_array("ftl_31")});

constexpr auto fml = std::to_array(
    {{"fml_0"},  {"fml_1"},  {"fml_2"},  {"fml_3"},  {"fml_4"},  {"fml_5"},  {"fml_6"},  {"fml_7"},
     {"fml_8"},  {"fml_9"},  {"fml_10"}, {"fml_11"}, {"fml_12"}, {"fml_13"}, {"fml_14"}, {"fml_15"},
     {"fml_16"}, {"fml_17"}, {"fml_18"}, {"fml_19"}, {"fml_20"}, {"fml_21"}, {"fml_22"}, {"fml_23"},
     {"fml_24"}, {"fml_25"}, {"fml_26"}, {"fml_27"}, {"fml_28"}, {"fml_29"}, {"fml_30"}, std::to_array("fml_31")});

constexpr auto sl = std::to_array(
    {{"sl_0"},  {"sl_1"},  {"sl_2"},  {"sl_3"},  {"sl_4"},  {"sl_5"},  {"sl_6"},  {"sl_7"},
     {"sl_8"},  {"sl_9"},  {"sl_10"}, {"sl_11"}, {"sl_12"}, {"sl_13"}, {"sl_14"}, {"sl_15"},
     {"sl_16"}, {"sl_17"}, {"sl_18"}, {"sl_19"}, {"sl_20"}, {"sl_21"}, {"sl_22"}, {"sl_23"},
     {"sl_24"}, {"sl_25"}, {"sl_26"}, {"sl_27"}, {"sl_28"}, {"sl_29"}, {"sl_30"}, std::to_array("sl_31")});

constexpr auto xsl = std::to_array(
    {{"xsl_0"},  {"xsl_1"},  {"xsl_2"},  {"xsl_3"},  {"xsl_4"},  {"xsl_5"},  {"xsl_6"},  {"xsl_7"},
     {"xsl_8"},  {"xsl_9"},  {"xsl_10"}, {"xsl_11"}, {"xsl_12"}, {"xsl_13"}, {"xsl_14"}, {"xsl_15"},
     {"xsl_16"}, {"xsl_17"}, {"xsl_18"}, {"xsl_19"}, {"xsl_20"}, {"xsl_21"}, {"xsl_22"}, {"xsl_23"},
     {"xsl_24"}, {"xsl_25"}, {"xsl_26"}, {"xsl_27"}, {"xsl_28"}, {"xsl_29"}, {"xsl_30"}, std::to_array("xsl_31")});

constexpr auto xml = std::to_array(
    {{"xml_0"},  {"xml_1"},  {"xml_2"},  {"xml_3"},  {"xml_4"},  {"xml_5"},  {"xml_6"},  {"xml_7"},
     {"xml_8"},  {"xml_9"},  {"xml_10"}, {"xml_11"}, {"xml_12"}, {"xml_13"}, {"xml_14"}, {"xml_15"},
     {"xml_16"}, {"xml_17"}, {"xml_18"}, {"xml_19"}, {"xml_20"}, {"xml_21"}, {"xml_22"}, {"xml_23"},
     {"xml_24"}, {"xml_25"}, {"xml_26"}, {"xml_27"}, {"xml_28"}, {"xml_29"}, {"xml_30"}, std::to_array("xml_31")});

constexpr auto ql = std::to_array(
    {{"ql_0"},  {"ql_1"},  {"ql_2"},  {"ql_3"},  {"ql_4"},  {"ql_5"},  {"ql_6"},  {"ql_7"},
     {"ql_8"},  {"ql_9"},  {"ql_10"}, {"ql_11"}, {"ql_12"}, {"ql_13"}, {"ql_14"}, {"ql_15"},
     {"ql_16"}, {"ql_17"}, {"ql_18"}, {"ql_19"}, {"ql_20"}, {"ql_21"}, {"ql_22"}, {"ql_23"},
     {"ql_24"}, {"ql_25"}, {"ql_26"}, {"ql_27"}, {"ql_28"}, {"ql_29"}, {"ql_30"}, std::to_array("ql_31")});

constexpr auto fl = std::to_array(
    {{"fl_0"},  {"fl_1"},  {"fl_2"},  {"fl_3"},  {"fl_4"},  {"fl_5"},  {"fl_6"},  {"fl_7"},
     {"fl_8"},  {"fl_9"},  {"fl_10"}, {"fl_11"}, {"fl_12"}, {"fl_13"}, {"fl_14"}, {"fl_15"},
     {"fl_16"}, {"fl_17"}, {"fl_18"}, {"fl_19"}, {"fl_20"}, {"fl_21"}, {"fl_22"}, {"fl_23"},
     {"fl_24"}, {"fl_25"}, {"fl_26"}, {"fl_27"}, {"fl_28"}, {"fl_29"}, {"fl_30"}, std::to_array("fl_31")});

constexpr auto gl = std::to_array(
    {{"gl_0"},  {"gl_1"},  {"gl_2"},  {"gl_3"},  {"gl_4"},  {"gl_5"},  {"gl_6"},  {"gl_7"},
     {"gl_8"},  {"gl_9"},  {"gl_10"}, {"gl_11"}, {"gl_12"}, {"gl_13"}, {"gl_14"}, {"gl_15"},
     {"gl_16"}, {"gl_17"}, {"gl_18"}, {"gl_19"}, {"gl_20"}, {"gl_21"}, {"gl_22"}, {"gl_23"},
     {"gl_24"}, {"gl_25"}, {"gl_26"}, {"gl_27"}, {"gl_28"}, {"gl_29"}, {"gl_30"}, std::to_array("gl_31")});

// right channel

constexpr auto ftr = std::to_array(
    {{"ftr_0"},  {"ftr_1"},  {"ftr_2"},  {"ftr_3"},  {"ftr_4"},  {"ftr_5"},  {"ftr_6"},  {"ftr_7"},
     {"ftr_8"},  {"ftr_9"},  {"ftr_10"}, {"ftr_11"}, {"ftr_12"}, {"ftr_13"}, {"ftr_14"}, {"ftr_15"},
     {"ftr_16"}, {"ftr_17"}, {"ftr_18"}, {"ftr_19"}, {"ftr_20"}, {"ftr_21"}, {"ftr_22"}, {"ftr_23"},
     {"ftr_24"}, {"ftr_25"}, {"ftr_26"}, {"ftr_27"}, {"ftr_28"}, {"ftr_29"}, {"ftr_30"}, std::to_array("ftr_31")});

constexpr auto fmr = std::to_array(
    {{"fmr_0"},  {"fmr_1"},  {"fmr_2"},  {"fmr_3"},  {"fmr_4"},  {"fmr_5"},  {"fmr_6"},  {"fmr_7"},
     {"fmr_8"},  {"fmr_9"},  {"fmr_10"}, {"fmr_11"}, {"fmr_12"}, {"fmr_13"}, {"fmr_14"}, {"fmr_15"},
     {"fmr_16"}, {"fmr_17"}, {"fmr_18"}, {"fmr_19"}, {"fmr_20"}, {"fmr_21"}, {"fmr_22"}, {"fmr_23"},
     {"fmr_24"}, {"fmr_25"}, {"fmr_26"}, {"fmr_27"}, {"fmr_28"}, {"fmr_29"}, {"fmr_30"}, std::to_array("fmr_31")});

constexpr auto sr = std::to_array(
    {{"sr_0"},  {"sr_1"},  {"sr_2"},  {"sr_3"},  {"sr_4"},  {"sr_5"},  {"sr_6"},  {"sr_7"},
     {"sr_8"},  {"sr_9"},  {"sr_10"}, {"sr_11"}, {"sr_12"}, {"sr_13"}, {"sr_14"}, {"sr_15"},
     {"sr_16"}, {"sr_17"}, {"sr_18"}, {"sr_19"}, {"sr_20"}, {"sr_21"}, {"sr_22"}, {"sr_23"},
     {"sr_24"}, {"sr_25"}, {"sr_26"}, {"sr_27"}, {"sr_28"}, {"sr_29"}, {"sr_30"}, std::to_array("sr_31")});

constexpr auto xsr = std::to_array(
    {{"xsr_0"},  {"xsr_1"},  {"xsr_2"},  {"xsr_3"},  {"xsr_4"},  {"xsr_5"},  {"xsr_6"},  {"xsr_7"},
     {"xsr_8"},  {"xsr_9"},  {"xsr_10"}, {"xsr_11"}, {"xsr_12"}, {"xsr_13"}, {"xsr_14"}, {"xsr_15"},
     {"xsr_16"}, {"xsr_17"}, {"xsr_18"}, {"xsr_19"}, {"xsr_20"}, {"xsr_21"}, {"xsr_22"}, {"xsr_23"},
     {"xsr_24"}, {"xsr_25"}, {"xsr_26"}, {"xsr_27"}, {"xsr_28"}, {"xsr_29"}, {"xsr_30"}, std::to_array("xsr_31")});

constexpr auto xmr = std::to_array(
    {{"xmr_0"},  {"xmr_1"},  {"xmr_2"},  {"xmr_3"},  {"xmr_4"},  {"xmr_5"},  {"xmr_6"},  {"xmr_7"},
     {"xmr_8"},  {"xmr_9"},  {"xmr_10"}, {"xmr_11"}, {"xmr_12"}, {"xmr_13"}, {"xmr_14"}, {"xmr_15"},
     {"xmr_16"}, {"xmr_17"}, {"xmr_18"}, {"xmr_19"}, {"xmr_20"}, {"xmr_21"}, {"xmr_22"}, {"xmr_23"},
     {"xmr_24"}, {"xmr_25"}, {"xmr_26"}, {"xmr_27"}, {"xmr_28"}, {"xmr_29"}, {"xmr_30"}, std::to_array("xmr_31")});

constexpr auto qr = std::to_array(
    {{"qr_0"},  {"qr_1"},  {"qr_2"},  {"qr_3"},  {"qr_4"},  {"qr_5"},  {"qr_6"},  {"qr_7"},
     {"qr_8"},  {"qr_9"},  {"qr_10"}, {"qr_11"}, {"qr_12"}, {"qr_13"}, {"qr_14"}, {"qr_15"},
     {"qr_16"}, {"qr_17"}, {"qr_18"}, {"qr_19"}, {"qr_20"}, {"qr_21"}, {"qr_22"}, {"qr_23"},
     {"qr_24"}, {"qr_25"}, {"qr_26"}, {"qr_27"}, {"qr_28"}, {"qr_29"}, {"qr_30"}, std::to_array("qr_31")});

constexpr auto fr = std::to_array(
    {{"fr_0"},  {"fr_1"},  {"fr_2"},  {"fr_3"},  {"fr_4"},  {"fr_5"},  {"fr_6"},  {"fr_7"},
     {"fr_8"},  {"fr_9"},  {"fr_10"}, {"fr_11"}, {"fr_12"}, {"fr_13"}, {"fr_14"}, {"fr_15"},
     {"fr_16"}, {"fr_17"}, {"fr_18"}, {"fr_19"}, {"fr_20"}, {"fr_21"}, {"fr_22"}, {"fr_23"},
     {"fr_24"}, {"fr_25"}, {"fr_26"}, {"fr_27"}, {"fr_28"}, {"fr_29"}, {"fr_30"}, std::to_array("fr_31")});

constexpr auto gr = std::to_array(
    {{"gr_0"},  {"gr_1"},  {"gr_2"},  {"gr_3"},  {"gr_4"},  {"gr_5"},  {"gr_6"},  {"gr_7"},
     {"gr_8"},  {"gr_9"},  {"gr_10"}, {"gr_11"}, {"gr_12"}, {"gr_13"}, {"gr_14"}, {"gr_15"},
     {"gr_16"}, {"gr_17"}, {"gr_18"}, {"gr_19"}, {"gr_20"}, {"gr_21"}, {"gr_22"}, {"gr_23"},
     {"gr_24"}, {"gr_25"}, {"gr_26"}, {"gr_27"}, {"gr_28"}, {"gr_29"}, {"gr_30"}, std::to_array("gr_31")});

}  // namespace tags::equalizer