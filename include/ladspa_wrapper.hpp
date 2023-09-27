/*
 *  Copyright © 2023 Torge Matthies
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

#include <dlfcn.h>
#include <ladspa.h>
#include <cassert>
#include <cstring>
#include <span>
#include <string>
#include <tuple>
#include <unordered_map>
#include "config.h"
#include "string_literal_wrapper.hpp"
#include "util.hpp"

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

  void set_control_port_value(uint index, float value);
  void set_control_port_value(const std::string& symbol, float value);
  auto set_control_port_value_clamp(uint index, float value) -> float;
  auto set_control_port_value_clamp(const std::string& symbol, float value) -> float;

  [[nodiscard]] auto found_plugin() const -> bool { return found; }
  [[nodiscard]] auto has_instance() const -> bool { return instance != nullptr; }
  [[nodiscard]] auto get_rate() const -> uint { return rate; }

#define set_from_gsettings(this, settings, setting_name, setting_type, val_type, port_name)           \
  do {                                                                                                \
    float value = static_cast<float>(g_settings_get_##setting_type((settings), (setting_name)));      \
    float actual_value = (this)->set_control_port_value_clamp((port_name), value);                    \
    if (actual_value != value && !(std::isnan(actual_value) && std::isnan(value)))                    \
      g_settings_set_##setting_type((settings), (setting_name), static_cast<val_type>(actual_value)); \
  } while (0)

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_bool(GSettings* settings) {
    set_from_gsettings(this, settings, gkey_wrapper.msg.data(), boolean, gboolean, key_wrapper.msg.data());

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto* self = static_cast<LadspaWrapper*>(user_data);
                       set_from_gsettings(self, settings, key, boolean, gboolean, key_wrapper.msg.data());
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_enum(GSettings* settings) {
    set_from_gsettings(this, settings, gkey_wrapper.msg.data(), enum, gint, key_wrapper.msg.data());

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto* self = static_cast<LadspaWrapper*>(user_data);
                       set_from_gsettings(self, settings, key, enum, gint, key_wrapper.msg.data());
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_int(GSettings* settings) {
    set_from_gsettings(this, settings, gkey_wrapper.msg.data(), int, gint, key_wrapper.msg.data());

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto* self = static_cast<LadspaWrapper*>(user_data);
                       set_from_gsettings(self, settings, key, int, gint, key_wrapper.msg.data());
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  void bind_key_double(GSettings* settings) {
    set_from_gsettings(this, settings, gkey_wrapper.msg.data(), double, gdouble, key_wrapper.msg.data());

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto* self = static_cast<LadspaWrapper*>(user_data);
                       set_from_gsettings(self, settings, key, double, gdouble, key_wrapper.msg.data());
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper, bool lower_bound = true>
  void bind_key_double_db_exponential(GSettings* settings) {
    auto key_v = g_settings_get_double(settings, gkey_wrapper.msg.data());

    auto val = static_cast<float>(key_v);

    auto clamped = (!lower_bound && key_v <= util::minimum_db_d_level) ? -std::numeric_limits<float>::infinity() : val;

    float new_v = set_control_port_value_clamp(key_wrapper.msg.data(), clamped);

    if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped))) {
      g_settings_set_double(settings, gkey_wrapper.msg.data(), new_v);
    }

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto* self = static_cast<LadspaWrapper*>(user_data);

                       auto key_v = g_settings_get_double(settings, gkey_wrapper.msg.data());

                       auto val = static_cast<float>(key_v);

                       auto clamped = (!lower_bound && key_v <= util::minimum_db_d_level)
                                          ? -std::numeric_limits<float>::infinity()
                                          : val;

                       float new_v = self->set_control_port_value_clamp(key_wrapper.msg.data(), clamped);

                       if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped)))
                         g_settings_set_double(settings, gkey_wrapper.msg.data(), new_v);
                     }),
                     this);
  }

  template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper, bool lower_bound = true>
  void bind_key_double_db(GSettings* settings) {
    auto key_v = g_settings_get_double(settings, gkey_wrapper.msg.data());

    auto linear_v = static_cast<float>(util::db_to_linear(key_v));

    auto clamped_v = (!lower_bound && linear_v <= util::minimum_db_d_level) ? 0.0F : linear_v;

    float new_v = set_control_port_value_clamp(key_wrapper.msg.data(), clamped_v);

    if (new_v != clamped_v && !(std::isnan(new_v) && std::isnan(clamped_v))) {
      g_settings_set_double(settings, gkey_wrapper.msg.data(), util::linear_to_db(new_v));
    }

    g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
                     G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                       auto* self = static_cast<LadspaWrapper*>(user_data);

                       auto key_v = g_settings_get_double(settings, gkey_wrapper.msg.data());

                       auto linear_v = static_cast<float>(util::db_to_linear(key_v));

                       auto clamped_v = (!lower_bound && key_v <= util::minimum_db_d_level) ? 0.0F : linear_v;

                       float new_v = self->set_control_port_value_clamp(key_wrapper.msg.data(), clamped_v);

                       if (new_v != clamped_v && !(std::isnan(new_v) && std::isnan(clamped_v)))
                         g_settings_set_double(settings, gkey_wrapper.msg.data(), util::linear_to_db(new_v));
                     }),
                     this);
  }

#undef set_from_gsettings

  uint n_samples = 0U;

 private:
  std::string plugin_name;

  void* dl_handle = nullptr;

  const LADSPA_Descriptor* descriptor = nullptr;

  LADSPA_Handle instance = nullptr;

  bool found = false;
  bool active = false;

  uint rate = 0U;

  LADSPA_Data* control_ports = nullptr;
  bool* control_ports_initialized = nullptr;

  std::unordered_map<std::string, unsigned long> map_cp_name_to_idx = std::unordered_map<std::string, unsigned long>();
};

}  // namespace ladspa
