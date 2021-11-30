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

namespace tags::equalizer {

// bands

constexpr char band_mode[][13] = {{"band0-mode"},  {"band1-mode"},  {"band2-mode"},  {"band3-mode"},  {"band4-mode"},
                                  {"band5-mode"},  {"band6-mode"},  {"band7-mode"},  {"band8-mode"},  {"band9-mode"},
                                  {"band10-mode"}, {"band11-mode"}, {"band12-mode"}, {"band13-mode"}, {"band14-mode"},
                                  {"band15-mode"}, {"band16-mode"}, {"band17-mode"}, {"band18-mode"}, {"band19-mode"},
                                  {"band20-mode"}, {"band21-mode"}, {"band22-mode"}, {"band23-mode"}, {"band24-mode"},
                                  {"band25-mode"}, {"band26-mode"}, {"band27-mode"}, {"band28-mode"}, {"band29-mode"},
                                  {"band30-mode"}, {"band31-mode"}};

constexpr char band_type[][13] = {{"band0-type"},  {"band1-type"},  {"band2-type"},  {"band3-type"},  {"band4-type"},
                                  {"band5-type"},  {"band6-type"},  {"band7-type"},  {"band8-type"},  {"band9-type"},
                                  {"band10-type"}, {"band11-type"}, {"band12-type"}, {"band13-type"}, {"band14-type"},
                                  {"band15-type"}, {"band16-type"}, {"band17-type"}, {"band18-type"}, {"band19-type"},
                                  {"band20-type"}, {"band21-type"}, {"band22-type"}, {"band23-type"}, {"band24-type"},
                                  {"band25-type"}, {"band26-type"}, {"band27-type"}, {"band28-type"}, {"band29-type"},
                                  {"band30-type"}, {"band31-type"}};

constexpr char band_frequency[][18] = {
    {"band0-frequency"},  {"band1-frequency"},  {"band2-frequency"},  {"band3-frequency"},  {"band4-frequency"},
    {"band5-frequency"},  {"band6-frequency"},  {"band7-frequency"},  {"band8-frequency"},  {"band9-frequency"},
    {"band10-frequency"}, {"band11-frequency"}, {"band12-frequency"}, {"band13-frequency"}, {"band14-frequency"},
    {"band15-frequency"}, {"band16-frequency"}, {"band17-frequency"}, {"band18-frequency"}, {"band19-frequency"},
    {"band20-frequency"}, {"band21-frequency"}, {"band22-frequency"}, {"band23-frequency"}, {"band24-frequency"},
    {"band25-frequency"}, {"band26-frequency"}, {"band27-frequency"}, {"band28-frequency"}, {"band29-frequency"},
    {"band30-frequency"}, {"band31-frequency"}};

constexpr char band_q[][10] = {
    {"band0-q"},  {"band1-q"},  {"band2-q"},  {"band3-q"},  {"band4-q"},  {"band5-q"},  {"band6-q"},  {"band7-q"},
    {"band8-q"},  {"band9-q"},  {"band10-q"}, {"band11-q"}, {"band12-q"}, {"band13-q"}, {"band14-q"}, {"band15-q"},
    {"band16-q"}, {"band17-q"}, {"band18-q"}, {"band19-q"}, {"band20-q"}, {"band21-q"}, {"band22-q"}, {"band23-q"},
    {"band24-q"}, {"band25-q"}, {"band26-q"}, {"band27-q"}, {"band28-q"}, {"band29-q"}, {"band30-q"}, {"band31-q"}};

// left channel

constexpr char fml[][8] = {{"fml_0"},  {"fml_1"},  {"fml_2"},  {"fml_3"},  {"fml_4"},  {"fml_5"},  {"fml_6"},
                           {"fml_7"},  {"fml_8"},  {"fml_9"},  {"fml_10"}, {"fml_11"}, {"fml_12"}, {"fml_13"},
                           {"fml_14"}, {"fml_15"}, {"fml_16"}, {"fml_17"}, {"fml_18"}, {"fml_19"}, {"fml_20"},
                           {"fml_21"}, {"fml_22"}, {"fml_23"}, {"fml_24"}, {"fml_25"}, {"fml_26"}, {"fml_27"},
                           {"fml_28"}, {"fml_29"}, {"fml_30"}, {"fml_31"}};

constexpr char ftl[][8] = {{"ftl_0"},  {"ftl_1"},  {"ftl_2"},  {"ftl_3"},  {"ftl_4"},  {"ftl_5"},  {"ftl_6"},
                           {"ftl_7"},  {"ftl_8"},  {"ftl_9"},  {"ftl_10"}, {"ftl_11"}, {"ftl_12"}, {"ftl_13"},
                           {"ftl_14"}, {"ftl_15"}, {"ftl_16"}, {"ftl_17"}, {"ftl_18"}, {"ftl_19"}, {"ftl_20"},
                           {"ftl_21"}, {"ftl_22"}, {"ftl_23"}, {"ftl_24"}, {"ftl_25"}, {"ftl_26"}, {"ftl_27"},
                           {"ftl_28"}, {"ftl_29"}, {"ftl_30"}, {"ftl_31"}};

constexpr char ql[][7] = {{"ql_0"},  {"ql_1"},  {"ql_2"},  {"ql_3"},  {"ql_4"},  {"ql_5"},  {"ql_6"},  {"ql_7"},
                          {"ql_8"},  {"ql_9"},  {"ql_10"}, {"ql_11"}, {"ql_12"}, {"ql_13"}, {"ql_14"}, {"ql_15"},
                          {"ql_16"}, {"ql_17"}, {"ql_18"}, {"ql_19"}, {"ql_20"}, {"ql_21"}, {"ql_22"}, {"ql_23"},
                          {"ql_24"}, {"ql_25"}, {"ql_26"}, {"ql_27"}, {"ql_28"}, {"ql_29"}, {"ql_30"}, {"ql_31"}};

constexpr char fl[][7] = {{"fl_0"},  {"fl_1"},  {"fl_2"},  {"fl_3"},  {"fl_4"},  {"fl_5"},  {"fl_6"},  {"fl_7"},
                          {"fl_8"},  {"fl_9"},  {"fl_10"}, {"fl_11"}, {"fl_12"}, {"fl_13"}, {"fl_14"}, {"fl_15"},
                          {"fl_16"}, {"fl_17"}, {"fl_18"}, {"fl_19"}, {"fl_20"}, {"fl_21"}, {"fl_22"}, {"fl_23"},
                          {"fl_24"}, {"fl_25"}, {"fl_26"}, {"fl_27"}, {"fl_28"}, {"fl_29"}, {"fl_30"}, {"fl_31"}};

// right channel

constexpr char fmr[][8] = {{"fmr_0"},  {"fmr_1"},  {"fmr_2"},  {"fmr_3"},  {"fmr_4"},  {"fmr_5"},  {"fmr_6"},
                           {"fmr_7"},  {"fmr_8"},  {"fmr_9"},  {"fmr_10"}, {"fmr_11"}, {"fmr_12"}, {"fmr_13"},
                           {"fmr_14"}, {"fmr_15"}, {"fmr_16"}, {"fmr_17"}, {"fmr_18"}, {"fmr_19"}, {"fmr_20"},
                           {"fmr_21"}, {"fmr_22"}, {"fmr_23"}, {"fmr_24"}, {"fmr_25"}, {"fmr_26"}, {"fmr_27"},
                           {"fmr_28"}, {"fmr_29"}, {"fmr_30"}, {"fmr_31"}};

constexpr char ftr[][8] = {{"ftr_0"},  {"ftr_1"},  {"ftr_2"},  {"ftr_3"},  {"ftr_4"},  {"ftr_5"},  {"ftr_6"},
                           {"ftr_7"},  {"ftr_8"},  {"ftr_9"},  {"ftr_10"}, {"ftr_11"}, {"ftr_12"}, {"ftr_13"},
                           {"ftr_14"}, {"ftr_15"}, {"ftr_16"}, {"ftr_17"}, {"ftr_18"}, {"ftr_19"}, {"ftr_20"},
                           {"ftr_21"}, {"ftr_22"}, {"ftr_23"}, {"ftr_24"}, {"ftr_25"}, {"ftr_26"}, {"ftr_27"},
                           {"ftr_28"}, {"ftr_29"}, {"ftr_30"}, {"ftr_31"}};

constexpr char qr[][7] = {{"qr_0"},  {"qr_1"},  {"qr_2"},  {"qr_3"},  {"qr_4"},  {"qr_5"},  {"qr_6"},  {"qr_7"},
                          {"qr_8"},  {"qr_9"},  {"qr_10"}, {"qr_11"}, {"qr_12"}, {"qr_13"}, {"qr_14"}, {"qr_15"},
                          {"qr_16"}, {"qr_17"}, {"qr_18"}, {"qr_19"}, {"qr_20"}, {"qr_21"}, {"qr_22"}, {"qr_23"},
                          {"qr_24"}, {"qr_25"}, {"qr_26"}, {"qr_27"}, {"qr_28"}, {"qr_29"}, {"qr_30"}, {"qr_31"}};

constexpr char fr[][7] = {{"fr_0"},  {"fr_1"},  {"fr_2"},  {"fr_3"},  {"fr_4"},  {"fr_5"},  {"fr_6"},  {"fr_7"},
                          {"fr_8"},  {"fr_9"},  {"fr_10"}, {"fr_11"}, {"fr_12"}, {"fr_13"}, {"fr_14"}, {"fr_15"},
                          {"fr_16"}, {"fr_17"}, {"fr_18"}, {"fr_19"}, {"fr_20"}, {"fr_21"}, {"fr_22"}, {"fr_23"},
                          {"fr_24"}, {"fr_25"}, {"fr_26"}, {"fr_27"}, {"fr_28"}, {"fr_29"}, {"fr_30"}, {"fr_31"}};

}  // namespace tags::equalizer