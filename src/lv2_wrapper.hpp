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

#include <lilv/lilv.h>
#include <lv2/buf-size/buf-size.h>
#include <lv2/core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/log/log.h>
#include <lv2/ui/ui.h>
#include <lv2/urid/urid.h>
#include <sys/types.h>
#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <mutex>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "lv2_ui.hpp"

namespace lv2 {

using namespace std::string_literals;

#define LV2_UI_makeResident LV2_UI_PREFIX "makeResident"

#define LV2_UI_makeSONameResident LV2_UI_PREFIX "makeSONameResident"

enum class PortType { TYPE_CONTROL, TYPE_AUDIO, TYPE_ATOM };

struct Port {
  PortType type;  // Datatype

  uint index;  // Port index

  std::string name;

  std::string symbol;

  float value = 0.0F;  // Control value (if applicable)

  float min = -std::numeric_limits<float>::infinity();

  float max = std::numeric_limits<float>::infinity();

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

  static constexpr auto min_quantum = 32;
  static constexpr auto max_quantum = 8192;

  const std::array<const LV2_Feature, 1U> static_features{{{.URI = LV2_BUF_SIZE__boundedBlockLength, .data = nullptr}}};

  std::unordered_map<LV2_URID, std::string> map_urid_to_uri;

  std::vector<Port> ports;

  std::vector<std::function<void()>> sync_funcs;

  auto create_instance(const uint& rate) -> bool;

  void destroy_instance();

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

  void load_ui();

  void notify_ui();

  void update_ui();

  auto has_ui() -> bool;

  void close_ui();

  void ui_port_event(const uint& port_index, const float& value);

  void native_ui_to_database();

  auto get_plugin_uri() -> std::string;

  auto get_lilv_plugin() -> const LilvPlugin*;

  auto get_instance() -> LilvInstance*;

  static auto lv2_printf([[maybe_unused]] LV2_Log_Handle handle,
                         [[maybe_unused]] LV2_URID type,
                         const char* format,
                         ...) -> int;

  auto map_urid(const std::string& uri) -> LV2_URID;

 private:
  std::string plugin_uri;

  LilvWorld* world = nullptr;

  const LilvPlugin* plugin = nullptr;

  LilvInstance* instance = nullptr;

  NativeUi native_ui;

  uint n_ports = 0U;
  uint n_audio_in = 0U;
  uint n_audio_out = 0U;

  uint n_samples = 0U;

  uint rate = 0U;

  // Multiband compressor/gate use 1+8*7=57 control ports. Round up to 64.
  std::array<std::pair<size_t, uint>, 64> control_ports_cache;

  struct {
    struct {
      uint left, right;
    } in;
    struct {
      uint left, right;
    } probe;
    struct {
      uint left, right;
    } out;
  } data_ports;

  std::unordered_map<std::string, LV2_URID> map_uri_to_urid;

  std::mutex ui_mutex;

  void check_required_features();

  void create_ports();

  void connect_control_ports();
};

}  // namespace lv2
