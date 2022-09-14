/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#include <utility>
#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"
#include "tags_equalizer.hpp"

class Equalizer : public PluginBase {
 public:
  Equalizer(const std::string& tag,
            const std::string& schema,
            const std::string& schema_path,
            const std::string& schema_channel,
            const std::string& schema_channel_left_path,
            const std::string& schema_channel_right_path,
            PipeManager* pipe_manager);
  Equalizer(const Equalizer&) = delete;
  auto operator=(const Equalizer&) -> Equalizer& = delete;
  Equalizer(const Equalizer&&) = delete;
  auto operator=(const Equalizer&&) -> Equalizer& = delete;
  ~Equalizer() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  auto get_latency_seconds() -> float override;

  float latency_port_value = 0.0F;

 private:
  GSettings *settings_left = nullptr, *settings_right = nullptr;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  static constexpr uint max_bands = 32U;

  uint latency_n_frames = 0U;

  std::vector<gulong> gconnections_unified;

  template <size_t n>
  constexpr void bind_band() {
    using namespace tags::equalizer;

    lv2_wrapper->bind_key_enum<ftl[n], band_type[n]>(settings_left);
    lv2_wrapper->bind_key_enum<fml[n], band_mode[n]>(settings_left);
    lv2_wrapper->bind_key_enum<sl[n], band_slope[n]>(settings_left);

    lv2_wrapper->bind_key_bool<xsl[n], band_solo[n]>(settings_left);
    lv2_wrapper->bind_key_bool<xml[n], band_mute[n]>(settings_left);

    lv2_wrapper->bind_key_double<fl[n], band_frequency[n]>(settings_left);
    lv2_wrapper->bind_key_double<ql[n], band_q[n]>(settings_left);

    lv2_wrapper->bind_key_double_db<gl[n], band_gain[n]>(settings_left);

    // right channel

    lv2_wrapper->bind_key_enum<ftr[n], band_type[n]>(settings_right);
    lv2_wrapper->bind_key_enum<fmr[n], band_mode[n]>(settings_right);
    lv2_wrapper->bind_key_enum<sr[n], band_slope[n]>(settings_right);

    lv2_wrapper->bind_key_bool<xsr[n], band_solo[n]>(settings_right);
    lv2_wrapper->bind_key_bool<xmr[n], band_mute[n]>(settings_right);

    lv2_wrapper->bind_key_double<fr[n], band_frequency[n]>(settings_right);
    lv2_wrapper->bind_key_double<qr[n], band_q[n]>(settings_right);

    lv2_wrapper->bind_key_double_db<gr[n], band_gain[n]>(settings_right);
  }

  template <size_t... Ns>
  constexpr void bind_bands(std::index_sequence<Ns...> /*unused*/) {
    (bind_band<Ns>(), ...);
  }

  void on_split_channels();
};
