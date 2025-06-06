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

  // using genum = gint;

  // template <typename T>
  // void load_from_gsettings(GSettings* settings, const char* gkey, const char* port_name) {
  //   float value = 0.0F;

  //   if constexpr (std::is_same_v<T, double>) {
  //     value = static_cast<float>(g_settings_get_double(settings, gkey));
  //   } else if constexpr (std::is_same_v<T, int>) {
  //     value = static_cast<float>(g_settings_get_int(settings, gkey));
  //   } else if constexpr (std::is_same_v<T, bool>) {
  //     value = static_cast<float>(g_settings_get_boolean(settings, gkey));
  //   } else if constexpr (std::is_same_v<T, genum>) {
  //     value = static_cast<float>(g_settings_get_enum(settings, gkey));
  //   }

  //   auto actual_value = set_control_port_value_clamp(port_name, value);

  //   if (actual_value != value && !(std::isnan(actual_value) && std::isnan(value))) {
  //     if constexpr (std::is_same_v<T, double>) {
  //       g_settings_set_double(settings, gkey, static_cast<double>(actual_value));
  //     } else if constexpr (std::is_same_v<T, int>) {
  //       g_settings_set_int(settings, gkey, static_cast<int>(actual_value));
  //     } else if constexpr (std::is_same_v<T, bool>) {
  //       g_settings_set_boolean(settings, gkey, static_cast<gboolean>(actual_value));
  //     } else if constexpr (std::is_same_v<T, genum>) {
  //       g_settings_set_enum(settings, gkey, static_cast<genum>(actual_value));
  //     }
  //   }
  // }

  // template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  // void bind_key_bool(GSettings* settings) {
  //   load_from_gsettings<bool>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());

  //   g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto* self = static_cast<LadspaWrapper*>(user_data);

  //                      self->load_from_gsettings<bool>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());
  //                    }),
  //                    this);
  // }

  // template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  // void bind_key_enum(GSettings* settings) {
  //   load_from_gsettings<genum>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());

  //   g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto* self = static_cast<LadspaWrapper*>(user_data);

  //                      self->load_from_gsettings<genum>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());
  //                    }),
  //                    this);
  // }

  // template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  // void bind_key_int(GSettings* settings) {
  //   load_from_gsettings<int>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());

  //   g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto* self = static_cast<LadspaWrapper*>(user_data);

  //                      self->load_from_gsettings<int>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());
  //                    }),
  //                    this);
  // }

  // template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper>
  // void bind_key_double(GSettings* settings) {
  //   load_from_gsettings<double>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());

  //   g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto* self = static_cast<LadspaWrapper*>(user_data);

  //                      self->load_from_gsettings<double>(settings, gkey_wrapper.msg.data(), key_wrapper.msg.data());
  //                    }),
  //                    this);
  // }

  // template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper, bool lower_bound = true>
  // void bind_key_double_db_exponential(GSettings* settings) {
  //   const auto db_v = static_cast<float>(g_settings_get_double(settings, gkey_wrapper.msg.data()));

  //   const auto clamped =
  //       (!lower_bound && db_v <= util::minimum_db_level) ? -std::numeric_limits<float>::infinity() : db_v;

  //   const auto new_v = set_control_port_value_clamp(key_wrapper.msg.data(), clamped);

  //   // Update gsettings if the new value is valid and clamped/different from the original
  //   if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped))) {
  //     g_settings_set_double(settings, gkey_wrapper.msg.data(), static_cast<gdouble>(new_v));
  //   }

  //   g_signal_connect(settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
  //                    G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //                      auto* self = static_cast<LadspaWrapper*>(user_data);

  //                      const auto db_v = static_cast<float>(g_settings_get_double(settings,
  //                      gkey_wrapper.msg.data()));

  //                      const auto clamped = (!lower_bound && db_v <= util::minimum_db_level)
  //                                               ? -std::numeric_limits<float>::infinity()
  //                                               : db_v;

  //                      const auto new_v = self->set_control_port_value_clamp(key_wrapper.msg.data(), clamped);

  //                      // Update gsettings if the new value is valid and clamped/different from the original
  //                      if (new_v != clamped && !(std::isnan(new_v) && std::isnan(clamped)))
  //                        g_settings_set_double(settings, gkey_wrapper.msg.data(), static_cast<gdouble>(new_v));
  //                    }),
  //                    this);
  // }

  // template <StringLiteralWrapper key_wrapper, StringLiteralWrapper gkey_wrapper, bool lower_bound = true>
  // void bind_key_double_db(GSettings* settings) {
  //   const auto db_v = static_cast<float>(g_settings_get_double(settings, gkey_wrapper.msg.data()));

  //   const auto clamped_v = (!lower_bound && db_v <= util::minimum_db_level) ? 0.0F : util::db_to_linear(db_v);

  //   const auto new_v = set_control_port_value_clamp(key_wrapper.msg.data(), clamped_v);

  //   // Update gsettings if the new value is valid and clamped/different from the original
  //   if (new_v != clamped_v && !(std::isnan(new_v) && std::isnan(clamped_v))) {
  //     g_settings_set_double(settings, gkey_wrapper.msg.data(), static_cast<gdouble>(util::linear_to_db(new_v)));
  //   }

  //   g_signal_connect(
  //       settings, ("changed::"s + gkey_wrapper.msg.data()).c_str(),
  //       G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
  //         auto* self = static_cast<LadspaWrapper*>(user_data);

  //         const auto db_v = static_cast<float>(g_settings_get_double(settings, gkey_wrapper.msg.data()));

  //         const auto clamped_v = (!lower_bound && db_v <= util::minimum_db_level) ? 0.0F : util::db_to_linear(db_v);

  //         const auto new_v = self->set_control_port_value_clamp(key_wrapper.msg.data(), clamped_v);

  //         // Update gsettings if the new value is valid and clamped/different from the original
  //         if (new_v != clamped_v && !(std::isnan(new_v) && std::isnan(clamped_v)))
  //           g_settings_set_double(settings, gkey_wrapper.msg.data(),
  //           static_cast<gdouble>(util::linear_to_db(new_v)));
  //       }),
  //       this);
  // }

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
