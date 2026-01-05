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

#include "lv2_macros.hpp"

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)

#define BIND_BANDS_PROPERTY(lsp_key, property, class)                                                        \
  {                                                                                                          \
    BIND_LV2_PORT(lsp_key[0].data(), band0##property, setBand0##property, class ::band0##property##Changed); \
    BIND_LV2_PORT(lsp_key[1].data(), band1##property, setBand1##property, class ::band1##property##Changed); \
    BIND_LV2_PORT(lsp_key[2].data(), band2##property, setBand2##property, class ::band2##property##Changed); \
    BIND_LV2_PORT(lsp_key[3].data(), band3##property, setBand3##property, class ::band3##property##Changed); \
    BIND_LV2_PORT(lsp_key[4].data(), band4##property, setBand4##property, class ::band4##property##Changed); \
    BIND_LV2_PORT(lsp_key[5].data(), band5##property, setBand5##property, class ::band5##property##Changed); \
    BIND_LV2_PORT(lsp_key[6].data(), band6##property, setBand6##property, class ::band6##property##Changed); \
    BIND_LV2_PORT(lsp_key[7].data(), band7##property, setBand7##property, class ::band7##property##Changed); \
  }

#define BIND_BANDS_PROPERTY_DB(lsp_key, property, class, enforceLowerBound)                                    \
  {                                                                                                            \
    BIND_LV2_PORT_DB(lsp_key[0].data(), band0##property, setBand0##property, class ::band0##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[1].data(), band1##property, setBand1##property, class ::band1##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[2].data(), band2##property, setBand2##property, class ::band2##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[3].data(), band3##property, setBand3##property, class ::band3##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[4].data(), band4##property, setBand4##property, class ::band4##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[5].data(), band5##property, setBand5##property, class ::band5##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[6].data(), band6##property, setBand6##property, class ::band6##property##Changed, \
                     enforceLowerBound);                                                                       \
    BIND_LV2_PORT_DB(lsp_key[7].data(), band7##property, setBand7##property, class ::band7##property##Changed, \
                     enforceLowerBound);                                                                       \
  }

// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
