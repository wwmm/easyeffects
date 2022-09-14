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

#include <lilv/lilv.h>
#include <lv2/atom/atom.h>
#include <lv2/buf-size/buf-size.h>
#include <lv2/core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/log/log.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/options/options.h>
#include <lv2/parameters/parameters.h>
#include <array>
#include <span>
#include <unordered_map>
#include "string_literal_wrapper.hpp"
#include "util.hpp"

namespace lv2 {

using namespace std::string_literals;

enum PortType { TYPE_CONTROL, TYPE_AUDIO, TYPE_ATOM };

struct Port {
  PortType type;  // Datatype

  uint index;  // Port index

  std::string name;

  std::string symbol;

  float value;  // Control value (if applicable)

  bool is_input;  // True if an input port

  bool optional;  // True if the connection is optional
};

class Lv2Wrapper {
 public:
  Lv2Wrapper(const std::string& plugin_uri);
  Lv2Wrapper(const Lv2Wrapper&) = delete;
  auto operator=(const Lv2Wrapper&) -> Lv2Wrapper& = delete;
  Lv2Wrapper(const Lv2Wrapper&&) = delete;
  auto operator=(const Lv2Wrapper&&) -> Lv2Wrapper& = delete;
  virtual ~Lv2Wrapper();

  bool found_plugin = false;

  auto create_instance(const uint& rate) -> bool;

  void set_n_samples(const uint& value);

  [[nodiscard]] auto get_n_samples() const -> uint;

  [[nodiscard]] auto get_rate() const -> uint;

  void connect_data_ports(std::span<float>& left_in,
                          std::span<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out);

  void connect_data_ports(std::span<float>& left_in,
                          std::span<float>& right_in,
                          std::span<float>& left_out,
                          std::span<float>& right_out,
                          std::span<float>& probe_left,
                          std::span<float>& probe_right);

  void activate();

  void run() const;

  void deactivate();

  void set_control_port_value(const std::string& symbol, const float& value);

  auto get_control_port_value(const std::string& symbol) -> float;

  auto has_instance() -> bool;

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_bool(GSettings* settings) {
    set_control_port_value(key_wrapper.msg.data(),
                           static_cast<float>(g_settings_get_boolean(settings, gkey_wrapper.msg.data())));

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto self = static_cast<Lv2Wrapper*>(user_data);

                       self->set_control_port_value(key_wrapper.msg.data(),
                                                    static_cast<float>(g_settings_get_boolean(settings, key)));
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_enum(GSettings* settings) {
    set_control_port_value(key_wrapper.msg.data(),
                           static_cast<float>(g_settings_get_enum(settings, gkey_wrapper.msg.data())));

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto self = static_cast<Lv2Wrapper*>(user_data);

                       self->set_control_port_value(key_wrapper.msg.data(),
                                                    static_cast<float>(g_settings_get_enum(settings, key)));
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_int(GSettings* settings) {
    set_control_port_value(key_wrapper.msg.data(),
                           static_cast<float>(g_settings_get_int(settings, gkey_wrapper.msg.data())));

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto self = static_cast<Lv2Wrapper*>(user_data);

                       self->set_control_port_value(key_wrapper.msg.data(),
                                                    static_cast<float>(g_settings_get_int(settings, key)));
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_double(GSettings* settings) {
    set_control_port_value(key_wrapper.msg.data(),
                           static_cast<float>(g_settings_get_double(settings, gkey_wrapper.msg.data())));

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto self = static_cast<Lv2Wrapper*>(user_data);

                       self->set_control_port_value(key_wrapper.msg.data(),
                                                    static_cast<float>(g_settings_get_double(settings, key)));
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper, bool lower_bound = true>
  void bind_key_double_db(GSettings* settings) {
    auto key_v = g_settings_get_double(settings, gkey_wrapper.msg.data());

    auto linear_v =
        (!lower_bound && key_v <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(key_v));

    set_control_port_value(key_wrapper.msg.data(), linear_v);

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto self = static_cast<Lv2Wrapper*>(user_data);

                       auto key_v = g_settings_get_double(settings, gkey_wrapper.msg.data());

                       auto linear_v = (!lower_bound && key_v <= util::minimum_db_d_level)
                                           ? 0.0F
                                           : static_cast<float>(util::db_to_linear(key_v));

                       self->set_control_port_value(key_wrapper.msg.data(), linear_v);
                     }),
                     this);
  }

 private:
  std::string plugin_uri;

  LilvWorld* world = nullptr;

  const LilvPlugin* plugin = nullptr;

  LilvInstance* instance = nullptr;

  uint n_ports = 0U;
  uint n_audio_in = 0U;
  uint n_audio_out = 0U;

  uint n_samples = 0U;

  uint rate = 0U;

  std::vector<Port> ports;

  std::unordered_map<std::string, LV2_URID> map_uri_to_urid;
  std::unordered_map<LV2_URID, std::string> map_urid_to_uri;

  const std::array<const LV2_Feature, 1U> static_features{{{LV2_BUF_SIZE__boundedBlockLength, nullptr}}};

  void check_required_features();

  void create_ports();

  void connect_control_ports();

  auto map_urid(const std::string& uri) -> LV2_URID;
};

}  // namespace lv2
