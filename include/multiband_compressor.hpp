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

#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"
#include "tags_multiband_compressor.hpp"

class MultibandCompressor : public PluginBase {
 public:
  MultibandCompressor(const std::string& tag,
                      const std::string& schema,
                      const std::string& schema_path,
                      PipeManager* pipe_manager);
  MultibandCompressor(const MultibandCompressor&) = delete;
  auto operator=(const MultibandCompressor&) -> MultibandCompressor& = delete;
  MultibandCompressor(const MultibandCompressor&&) = delete;
  auto operator=(const MultibandCompressor&&) -> MultibandCompressor& = delete;
  ~MultibandCompressor() override;

  static constexpr uint n_bands = 8U;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  auto get_latency_seconds() -> float override;

  void update_probe_links() override;

  sigc::signal<void(const std::array<float, n_bands>)> reduction, envelope, curve, frequency_range;

  float latency_port_value = 0.0F;

  std::array<float, n_bands> frequency_range_end_port_array = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};
  std::array<float, n_bands> envelope_port_array = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};
  std::array<float, n_bands> curve_port_array = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};
  std::array<float, n_bands> reduction_port_array = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};

 private:
  uint latency_n_frames = 0U;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  std::vector<pw_proxy*> list_proxies;

  void update_sidechain_links(const std::string& key);

  template <size_t n>
  constexpr void bind_band() {
    using namespace tags::multiband_compressor;
    using namespace std::string_literals;

    if (n > 0U) {
      lv2_wrapper->bind_key_bool<cbe[n], band_enable[n]>(settings);

      lv2_wrapper->bind_key_double<sf[n], band_split_frequency[n]>(settings);
    }

    lv2_wrapper->bind_key_bool<sce[n], band_external_sidechain[n]>(settings);
    lv2_wrapper->bind_key_bool<sclc[n], band_lowcut_filter[n]>(settings);
    lv2_wrapper->bind_key_bool<schc[n], band_highcut_filter[n]>(settings);
    lv2_wrapper->bind_key_bool<ce[n], band_compressor_enable[n]>(settings);
    lv2_wrapper->bind_key_bool<bs[n], band_solo[n]>(settings);
    lv2_wrapper->bind_key_bool<bm[n], band_mute[n]>(settings);

    lv2_wrapper->bind_key_enum<scs[n], band_sidechain_source[n]>(settings);
    lv2_wrapper->bind_key_enum<scm[n], band_sidechain_mode[n]>(settings);
    lv2_wrapper->bind_key_enum<cm[n], band_compression_mode[n]>(settings);

    lv2_wrapper->bind_key_double<sla[n], band_sidechain_lookahead[n]>(settings);
    lv2_wrapper->bind_key_double<scr[n], band_sidechain_reactivity[n]>(settings);
    lv2_wrapper->bind_key_double<sclf[n], band_lowcut_filter_frequency[n]>(settings);
    lv2_wrapper->bind_key_double<schf[n], band_highcut_filter_frequency[n]>(settings);
    lv2_wrapper->bind_key_double<at[n], band_attack_time[n]>(settings);
    lv2_wrapper->bind_key_double<rt[n], band_release_time[n]>(settings);
    lv2_wrapper->bind_key_double<cr[n], band_ratio[n]>(settings);

    lv2_wrapper->bind_key_double_db<scp[n], band_sidechain_preamp[n]>(settings);
    lv2_wrapper->bind_key_double_db<al[n], band_attack_threshold[n]>(settings);
    lv2_wrapper->bind_key_double_db<kn[n], band_knee[n]>(settings);
    lv2_wrapper->bind_key_double_db<bth[n], band_boost_threshold[n]>(settings);
    lv2_wrapper->bind_key_double_db<bsa[n], band_boost_amount[n]>(settings);
    lv2_wrapper->bind_key_double_db<mk[n], band_makeup[n]>(settings);

    // This control can assume -inf
    lv2_wrapper->bind_key_double_db<rrl[n], band_release_threshold[n], false>(settings);

    gconnections.push_back(g_signal_connect(settings, ("changed::"s + band_external_sidechain[n].data()).c_str(),
                                            G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                              auto self = static_cast<MultibandCompressor*>(user_data);

                                              self->update_sidechain_links(key);
                                            }),
                                            this));
  }

  template <size_t... Ns>
  constexpr void bind_bands(std::index_sequence<Ns...> /*unused*/) {
    (bind_band<Ns>(), ...);
  }
};
