/**
 * Copyright © 2023-2026 Torge Matthies
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

#include <dlfcn.h>
#include <ladspa.h>
#include <sys/types.h>
#include <span>
#include <string>
#include <tuple>
#include <unordered_map>

namespace ladspa {

using namespace std::string_literals;

class LadspaWrapper {
 public:
  LadspaWrapper(const std::string& plugin_filename, const std::string& plugin_label);
  LadspaWrapper(const LadspaWrapper&) = delete;
  auto operator=(const LadspaWrapper&) -> LadspaWrapper& = delete;
  LadspaWrapper(const LadspaWrapper&&) = delete;
  auto operator=(const LadspaWrapper&&) -> LadspaWrapper& = delete;
  virtual ~LadspaWrapper();

  auto create_instance(uint rate) -> bool;

  void connect_data_ports(const std::span<const float>& left_in,
                          const std::span<const float>& right_in,
                          const std::span<float>& left_out,
                          const std::span<float>& right_out);
  void connect_data_ports(const std::span<const float>& left_in,
                          const std::span<const float>& right_in,
                          const std::span<float>& left_out,
                          const std::span<float>& right_out,
                          const std::span<const float>& probe_left,
                          const std::span<const float>& probe_right);

  void activate();
  void deactivate();

  void run() const;

  [[nodiscard]] auto get_control_port_count() const -> uint;
  [[nodiscard]] auto get_control_port_name(uint index) const -> std::string;
  [[nodiscard]] auto is_control_port_output(uint index) const -> bool;
  [[nodiscard]] auto get_control_port_range(uint index) const -> std::tuple<float, float>;
  [[nodiscard]] auto get_control_port_default(uint index) const -> float;
  [[nodiscard]] auto get_control_port_value(uint index) const -> float;
  [[nodiscard]] auto get_control_port_value(const std::string& symbol) const -> float;

  auto set_control_port_value_clamp(uint index, float value) -> float;
  auto set_control_port_value_clamp(const std::string& symbol, float value) -> float;

  [[nodiscard]] auto found_plugin() const -> bool { return found; }
  [[nodiscard]] auto has_instance() const -> bool { return instance != nullptr; }
  [[nodiscard]] auto get_rate() const -> uint { return rate; }

  uint n_samples = 0U;

 private:
  std::string plugin_name;

  void* dl_handle = nullptr;

  const LADSPA_Descriptor* descriptor = nullptr;

  LADSPA_Handle instance = nullptr;

  static constexpr unsigned long null_ul = (unsigned long)-1L;

  bool found = false;
  bool active = false;

  uint rate = 0U;

  LADSPA_Data* control_ports = nullptr;
  bool* control_ports_initialized = nullptr;

  std::unordered_map<std::string, unsigned long> map_cp_name_to_idx;
};

}  // namespace ladspa
