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
#define BIND_LV2_PORT(key, getter, setter, onChangedSignal)                                                      \
  {                                                                                                              \
    lv2_wrapper->set_control_port_value(key, static_cast<float>(settings->getter()));                            \
    lv2_wrapper->sync_funcs.emplace_back([&]() { settings->setter(lv2_wrapper->get_control_port_value(key)); }); \
    connect(settings, &onChangedSignal, [this]() {                                                               \
      if (this == nullptr || settings == nullptr || lv2_wrapper == nullptr) {                                    \
        return;                                                                                                  \
      }                                                                                                          \
      lv2_wrapper->set_control_port_value(key, static_cast<float>(settings->getter()));                          \
    });                                                                                                          \
  }

#define BIND_LV2_PORT_DB(key, getter, setter, onChangedSignal, enforceLowerBound)                               \
  {                                                                                                             \
    auto db_v = settings->getter();                                                                             \
    auto linear_v = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                   \
                        ? 0.0F                                                                                  \
                        : static_cast<float>(util::db_to_linear(db_v));                                         \
    lv2_wrapper->set_control_port_value(key, linear_v);                                                         \
    lv2_wrapper->sync_funcs.emplace_back([this]() {                                                             \
      const auto linear_v = lv2_wrapper->get_control_port_value(key);                                           \
      const auto db_v =                                                                                         \
          ((enforceLowerBound) & (linear_v == 0.0F)) ? util::minimum_db_d_level : util::linear_to_db(linear_v); \
      settings->setter(db_v);                                                                                   \
    });                                                                                                         \
    connect(settings, &onChangedSignal, [this]() {                                                              \
      if (this == nullptr || settings == nullptr || lv2_wrapper == nullptr) {                                   \
        return;                                                                                                 \
      }                                                                                                         \
      auto db_v = settings->getter();                                                                           \
      auto linear_v = ((enforceLowerBound) && db_v <= util::minimum_db_d_level)                                 \
                          ? 0.0F                                                                                \
                          : static_cast<float>(util::db_to_linear(db_v));                                       \
      lv2_wrapper->set_control_port_value(key, linear_v);                                                       \
    });                                                                                                         \
  }

#define BIND_LV2_PORT_INVERTED_BOOL(key, getter, setter, onChangedSignal)                           \
  {                                                                                                 \
    lv2_wrapper->set_control_port_value(key, static_cast<float>(!settings->getter()));              \
    lv2_wrapper->sync_funcs.emplace_back(                                                           \
        [&]() { settings->setter(!static_cast<bool>(lv2_wrapper->get_control_port_value(key))); }); \
    connect(settings, &onChangedSignal, [this]() {                                                  \
      if (this == nullptr || settings == nullptr || lv2_wrapper == nullptr) {                       \
        return;                                                                                     \
      }                                                                                             \
      lv2_wrapper->set_control_port_value(key, static_cast<float>(!settings->getter()));            \
    });                                                                                             \
  }
// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
