/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects
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

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_BAND_PORT(settings_obj, key, getter, setter, onChangedSignal)                                           \
  {                                                                                                                  \
    lv2_wrapper->set_control_port_value(key, static_cast<float>(settings_obj->getter()));                            \
    lv2_wrapper->sync_funcs.emplace_back([&]() { settings_obj->setter(lv2_wrapper->get_control_port_value(key)); }); \
    connect(settings_obj, &onChangedSignal, [this]() {                                                               \
      if (this == nullptr || settings_obj == nullptr || lv2_wrapper == nullptr) {                                    \
        return;                                                                                                      \
      }                                                                                                              \
      lv2_wrapper->set_control_port_value(key, static_cast<float>(settings_obj->getter()));                          \
    });                                                                                                              \
  }

#define BIND_BAND_PORT_DB(settings_obj, key, getter, setter, onChangedSignal, enforceLowerBound)                \
  {                                                                                                             \
    auto db_v = settings_obj->getter();                                                                         \
    auto linear_v = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                   \
                        ? 0.0F                                                                                  \
                        : static_cast<float>(util::db_to_linear(db_v));                                         \
    lv2_wrapper->set_control_port_value(key, linear_v);                                                         \
    lv2_wrapper->sync_funcs.emplace_back([this]() {                                                             \
      const auto linear_v = lv2_wrapper->get_control_port_value(key);                                           \
      const auto db_v =                                                                                         \
          ((enforceLowerBound) & (linear_v == 0.0F)) ? util::minimum_db_d_level : util::linear_to_db(linear_v); \
      settings_obj->setter(db_v);                                                                               \
    });                                                                                                         \
    connect(settings_obj, &onChangedSignal, [this]() {                                                          \
      if (this == nullptr || settings_obj == nullptr || lv2_wrapper == nullptr) {                               \
        return;                                                                                                 \
      }                                                                                                         \
      auto db_v = settings_obj->getter();                                                                       \
      auto linear_v = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                 \
                          ? 0.0F                                                                                \
                          : static_cast<float>(util::db_to_linear(db_v));                                       \
      lv2_wrapper->set_control_port_value(key, linear_v);                                                       \
    });                                                                                                         \
  }

#define BIND_BANDS_PROPERTY(settings_obj, lsp_key, property)                                \
  {                                                                                         \
    BIND_BAND_PORT(settings_obj, lsp_key[0].data(), band0##property, setBand0##property,    \
                   db::EqualizerChannel::band0##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[1].data(), band1##property, setBand1##property,    \
                   db::EqualizerChannel::band1##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[2].data(), band2##property, setBand2##property,    \
                   db::EqualizerChannel::band2##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[3].data(), band3##property, setBand3##property,    \
                   db::EqualizerChannel::band3##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[4].data(), band4##property, setBand4##property,    \
                   db::EqualizerChannel::band4##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[5].data(), band5##property, setBand5##property,    \
                   db::EqualizerChannel::band5##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[6].data(), band6##property, setBand6##property,    \
                   db::EqualizerChannel::band6##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[7].data(), band7##property, setBand7##property,    \
                   db::EqualizerChannel::band7##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[8].data(), band8##property, setBand8##property,    \
                   db::EqualizerChannel::band8##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[9].data(), band9##property, setBand9##property,    \
                   db::EqualizerChannel::band9##property##Changed);                         \
    BIND_BAND_PORT(settings_obj, lsp_key[10].data(), band10##property, setBand10##property, \
                   db::EqualizerChannel::band10##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[11].data(), band11##property, setBand11##property, \
                   db::EqualizerChannel::band11##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[12].data(), band12##property, setBand12##property, \
                   db::EqualizerChannel::band12##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[13].data(), band13##property, setBand13##property, \
                   db::EqualizerChannel::band13##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[14].data(), band14##property, setBand14##property, \
                   db::EqualizerChannel::band14##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[15].data(), band15##property, setBand15##property, \
                   db::EqualizerChannel::band15##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[16].data(), band16##property, setBand16##property, \
                   db::EqualizerChannel::band16##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[17].data(), band17##property, setBand17##property, \
                   db::EqualizerChannel::band17##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[18].data(), band18##property, setBand18##property, \
                   db::EqualizerChannel::band18##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[19].data(), band19##property, setBand19##property, \
                   db::EqualizerChannel::band19##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[20].data(), band20##property, setBand20##property, \
                   db::EqualizerChannel::band20##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[21].data(), band21##property, setBand21##property, \
                   db::EqualizerChannel::band21##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[22].data(), band22##property, setBand22##property, \
                   db::EqualizerChannel::band22##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[23].data(), band23##property, setBand23##property, \
                   db::EqualizerChannel::band23##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[24].data(), band24##property, setBand24##property, \
                   db::EqualizerChannel::band24##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[25].data(), band25##property, setBand25##property, \
                   db::EqualizerChannel::band25##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[26].data(), band26##property, setBand26##property, \
                   db::EqualizerChannel::band26##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[27].data(), band27##property, setBand27##property, \
                   db::EqualizerChannel::band27##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[28].data(), band28##property, setBand28##property, \
                   db::EqualizerChannel::band28##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[29].data(), band29##property, setBand29##property, \
                   db::EqualizerChannel::band29##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[30].data(), band30##property, setBand30##property, \
                   db::EqualizerChannel::band30##property##Changed);                        \
    BIND_BAND_PORT(settings_obj, lsp_key[31].data(), band31##property, setBand31##property, \
                   db::EqualizerChannel::band31##property##Changed);                        \
  }

#define BIND_BANDS_PROPERTY_DB(settings_obj, lsp_key, property, enforceLowerBound)             \
  {                                                                                            \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[0].data(), band0##property, setBand0##property,    \
                      db::EqualizerChannel::band0##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[1].data(), band1##property, setBand1##property,    \
                      db::EqualizerChannel::band1##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[2].data(), band2##property, setBand2##property,    \
                      db::EqualizerChannel::band2##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[3].data(), band3##property, setBand3##property,    \
                      db::EqualizerChannel::band3##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[4].data(), band4##property, setBand4##property,    \
                      db::EqualizerChannel::band4##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[5].data(), band5##property, setBand5##property,    \
                      db::EqualizerChannel::band5##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[6].data(), band6##property, setBand6##property,    \
                      db::EqualizerChannel::band6##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[7].data(), band7##property, setBand7##property,    \
                      db::EqualizerChannel::band7##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[8].data(), band8##property, setBand8##property,    \
                      db::EqualizerChannel::band8##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[9].data(), band9##property, setBand9##property,    \
                      db::EqualizerChannel::band9##property##Changed, enforceLowerBound);      \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[10].data(), band10##property, setBand10##property, \
                      db::EqualizerChannel::band10##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[11].data(), band11##property, setBand11##property, \
                      db::EqualizerChannel::band11##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[12].data(), band12##property, setBand12##property, \
                      db::EqualizerChannel::band12##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[13].data(), band13##property, setBand13##property, \
                      db::EqualizerChannel::band13##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[14].data(), band14##property, setBand14##property, \
                      db::EqualizerChannel::band14##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[15].data(), band15##property, setBand15##property, \
                      db::EqualizerChannel::band15##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[16].data(), band16##property, setBand16##property, \
                      db::EqualizerChannel::band16##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[17].data(), band17##property, setBand17##property, \
                      db::EqualizerChannel::band17##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[18].data(), band18##property, setBand18##property, \
                      db::EqualizerChannel::band18##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[19].data(), band19##property, setBand19##property, \
                      db::EqualizerChannel::band19##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[20].data(), band20##property, setBand20##property, \
                      db::EqualizerChannel::band20##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[21].data(), band21##property, setBand21##property, \
                      db::EqualizerChannel::band21##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[22].data(), band22##property, setBand22##property, \
                      db::EqualizerChannel::band22##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[23].data(), band23##property, setBand23##property, \
                      db::EqualizerChannel::band23##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[24].data(), band24##property, setBand24##property, \
                      db::EqualizerChannel::band24##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[25].data(), band25##property, setBand25##property, \
                      db::EqualizerChannel::band25##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[26].data(), band26##property, setBand26##property, \
                      db::EqualizerChannel::band26##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[27].data(), band27##property, setBand27##property, \
                      db::EqualizerChannel::band27##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[28].data(), band28##property, setBand28##property, \
                      db::EqualizerChannel::band28##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[29].data(), band29##property, setBand29##property, \
                      db::EqualizerChannel::band29##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[30].data(), band30##property, setBand30##property, \
                      db::EqualizerChannel::band30##property##Changed, enforceLowerBound);     \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[31].data(), band31##property, setBand31##property, \
                      db::EqualizerChannel::band31##property##Changed, enforceLowerBound);     \
  }

#define UNIFIED_BAND_PORT_BIND(settings_right, settings_left, getter, setter, onChangedSignal) \
  {                                                                                            \
    settings_right->setter(settings_left->getter());                                           \
    unified_mode_connections.push_back(connect(settings_left, &onChangedSignal, [this]() {     \
      if (this == nullptr || settings_right == nullptr || settings_left == nullptr) {          \
        return;                                                                                \
      }                                                                                        \
      settings_right->setter(settings_left->getter());                                         \
    }));                                                                                       \
  }

#define UNIFIED_BANDS_PROPERTY_BIND(settings_right, settings_left, property)                     \
  {                                                                                              \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band0##property, setBand0##property,   \
                           db::EqualizerChannel::band0##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band1##property, setBand1##property,   \
                           db::EqualizerChannel::band1##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band2##property, setBand2##property,   \
                           db::EqualizerChannel::band2##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band3##property, setBand3##property,   \
                           db::EqualizerChannel::band3##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band4##property, setBand4##property,   \
                           db::EqualizerChannel::band4##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band5##property, setBand5##property,   \
                           db::EqualizerChannel::band5##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band6##property, setBand6##property,   \
                           db::EqualizerChannel::band6##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band7##property, setBand7##property,   \
                           db::EqualizerChannel::band7##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band8##property, setBand8##property,   \
                           db::EqualizerChannel::band8##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band9##property, setBand9##property,   \
                           db::EqualizerChannel::band9##property##Changed);                      \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band10##property, setBand10##property, \
                           db::EqualizerChannel::band10##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band11##property, setBand11##property, \
                           db::EqualizerChannel::band11##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band12##property, setBand12##property, \
                           db::EqualizerChannel::band12##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band13##property, setBand13##property, \
                           db::EqualizerChannel::band13##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band14##property, setBand14##property, \
                           db::EqualizerChannel::band14##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band15##property, setBand15##property, \
                           db::EqualizerChannel::band15##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band16##property, setBand16##property, \
                           db::EqualizerChannel::band16##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band17##property, setBand17##property, \
                           db::EqualizerChannel::band17##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band18##property, setBand18##property, \
                           db::EqualizerChannel::band18##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band19##property, setBand19##property, \
                           db::EqualizerChannel::band19##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band20##property, setBand20##property, \
                           db::EqualizerChannel::band20##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band21##property, setBand21##property, \
                           db::EqualizerChannel::band21##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band22##property, setBand22##property, \
                           db::EqualizerChannel::band22##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band23##property, setBand23##property, \
                           db::EqualizerChannel::band23##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band24##property, setBand24##property, \
                           db::EqualizerChannel::band24##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band25##property, setBand25##property, \
                           db::EqualizerChannel::band25##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band26##property, setBand26##property, \
                           db::EqualizerChannel::band26##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band27##property, setBand27##property, \
                           db::EqualizerChannel::band27##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band28##property, setBand28##property, \
                           db::EqualizerChannel::band28##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band29##property, setBand29##property, \
                           db::EqualizerChannel::band29##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band30##property, setBand30##property, \
                           db::EqualizerChannel::band30##property##Changed);                     \
    UNIFIED_BAND_PORT_BIND(settings_right, settings_left, band31##property, setBand31##property, \
                           db::EqualizerChannel::band31##property##Changed);                     \
  }

#define RESET_BAND_PROPERTY(settings_obj, idx, property)                                     \
  {                                                                                          \
    settings_obj->setBand##idx##property(settings_obj->defaultBand##idx##property##Value()); \
  }

#define RESET_BANDS_PROPERTY(settings_obj, property) \
  {                                                  \
    RESET_BAND_PROPERTY(settings_obj, 0, property);  \
    RESET_BAND_PROPERTY(settings_obj, 1, property);  \
    RESET_BAND_PROPERTY(settings_obj, 2, property);  \
    RESET_BAND_PROPERTY(settings_obj, 3, property);  \
    RESET_BAND_PROPERTY(settings_obj, 4, property);  \
    RESET_BAND_PROPERTY(settings_obj, 5, property);  \
    RESET_BAND_PROPERTY(settings_obj, 6, property);  \
    RESET_BAND_PROPERTY(settings_obj, 7, property);  \
    RESET_BAND_PROPERTY(settings_obj, 8, property);  \
    RESET_BAND_PROPERTY(settings_obj, 9, property);  \
    RESET_BAND_PROPERTY(settings_obj, 10, property); \
    RESET_BAND_PROPERTY(settings_obj, 11, property); \
    RESET_BAND_PROPERTY(settings_obj, 12, property); \
    RESET_BAND_PROPERTY(settings_obj, 13, property); \
    RESET_BAND_PROPERTY(settings_obj, 14, property); \
    RESET_BAND_PROPERTY(settings_obj, 15, property); \
    RESET_BAND_PROPERTY(settings_obj, 16, property); \
    RESET_BAND_PROPERTY(settings_obj, 17, property); \
    RESET_BAND_PROPERTY(settings_obj, 18, property); \
    RESET_BAND_PROPERTY(settings_obj, 19, property); \
    RESET_BAND_PROPERTY(settings_obj, 20, property); \
    RESET_BAND_PROPERTY(settings_obj, 21, property); \
    RESET_BAND_PROPERTY(settings_obj, 22, property); \
    RESET_BAND_PROPERTY(settings_obj, 23, property); \
    RESET_BAND_PROPERTY(settings_obj, 24, property); \
    RESET_BAND_PROPERTY(settings_obj, 25, property); \
    RESET_BAND_PROPERTY(settings_obj, 26, property); \
    RESET_BAND_PROPERTY(settings_obj, 27, property); \
    RESET_BAND_PROPERTY(settings_obj, 28, property); \
    RESET_BAND_PROPERTY(settings_obj, 29, property); \
    RESET_BAND_PROPERTY(settings_obj, 30, property); \
    RESET_BAND_PROPERTY(settings_obj, 31, property); \
  }

// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
