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

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
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
