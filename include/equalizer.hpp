/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects
 *
 *  EasyEffectsis free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffectsis distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <utility>
#include "equalizer_tags.hpp"
#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"

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

  sigc::signal<void(const float&)> latency;

  float latency_port_value = 0.0F;

 private:
  GSettings *settings_left = nullptr, *settings_right = nullptr;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  static constexpr uint max_bands = 32U;

  uint latency_n_frames = 0U;

  std::vector<gulong> gconnections_split;

  template <size_t n>
  constexpr void bind_band() {
    using namespace tags::equalizer;

    lv2_wrapper->bind_key_enum<ftl[n]>(settings_left, band_type[n]);
    lv2_wrapper->bind_key_enum<fml[n]>(settings_left, band_mode[n]);
    lv2_wrapper->bind_key_enum<sl[n]>(settings_left, band_slope[n]);

    lv2_wrapper->bind_key_bool<xsl[n]>(settings_left, band_solo[n]);
    lv2_wrapper->bind_key_bool<xml[n]>(settings_left, band_mute[n]);

    lv2_wrapper->bind_key_double<fl[n]>(settings_left, band_frequency[n]);
    lv2_wrapper->bind_key_double<ql[n]>(settings_left, band_q[n]);

    lv2_wrapper->bind_key_double_db<gl[n]>(settings_left, band_gain[n]);

    // right channel

    lv2_wrapper->bind_key_enum<ftr[n]>(settings_right, band_type[n]);
    lv2_wrapper->bind_key_enum<fmr[n]>(settings_right, band_mode[n]);
    lv2_wrapper->bind_key_enum<sr[n]>(settings_right, band_slope[n]);

    lv2_wrapper->bind_key_bool<xsr[n]>(settings_right, band_solo[n]);
    lv2_wrapper->bind_key_bool<xmr[n]>(settings_right, band_mute[n]);

    lv2_wrapper->bind_key_double<fr[n]>(settings_right, band_frequency[n]);
    lv2_wrapper->bind_key_double<qr[n]>(settings_right, band_q[n]);

    lv2_wrapper->bind_key_double_db<gr[n]>(settings_right, band_gain[n]);
  }

  template <size_t... Ns>
  constexpr void bind_bands(std::index_sequence<Ns...>) {
    (bind_band<Ns>(), ...);
  }

  void on_split_channels();
};

#endif
