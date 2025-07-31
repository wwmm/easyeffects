/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects
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

#define BIND_BANDS_PROPERTY(settings_obj, lsp_key, property)                             \
  {                                                                                      \
    BIND_BAND_PORT(settings_obj, lsp_key[0].data(), band0##property, setBand0##property, \
                   db::EqualizerChannel::band0##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[1].data(), band1##property, setBand1##property, \
                   db::EqualizerChannel::band1##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[2].data(), band2##property, setBand2##property, \
                   db::EqualizerChannel::band2##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[3].data(), band3##property, setBand3##property, \
                   db::EqualizerChannel::band3##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[4].data(), band4##property, setBand4##property, \
                   db::EqualizerChannel::band4##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[5].data(), band5##property, setBand5##property, \
                   db::EqualizerChannel::band5##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[6].data(), band6##property, setBand6##property, \
                   db::EqualizerChannel::band6##property##Changed);                      \
    BIND_BAND_PORT(settings_obj, lsp_key[7].data(), band7##property, setBand7##property, \
                   db::EqualizerChannel::band7##property##Changed);                      \
  }

#define BIND_BANDS_PROPERTY_DB(settings_obj, lsp_key, property, enforceLowerBound)          \
  {                                                                                         \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[0].data(), band0##property, setBand0##property, \
                      db::EqualizerChannel::band0##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[1].data(), band1##property, setBand1##property, \
                      db::EqualizerChannel::band1##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[2].data(), band2##property, setBand2##property, \
                      db::EqualizerChannel::band2##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[3].data(), band3##property, setBand3##property, \
                      db::EqualizerChannel::band3##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[4].data(), band4##property, setBand4##property, \
                      db::EqualizerChannel::band4##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[5].data(), band5##property, setBand5##property, \
                      db::EqualizerChannel::band5##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[6].data(), band6##property, setBand6##property, \
                      db::EqualizerChannel::band6##property##Changed, enforceLowerBound);   \
    BIND_BAND_PORT_DB(settings_obj, lsp_key[7].data(), band7##property, setBand7##property, \
                      db::EqualizerChannel::band7##property##Changed, enforceLowerBound);   \
  }

// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)