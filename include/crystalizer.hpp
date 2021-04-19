/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

#include <thread>
#include <vector>
#include "fir_filter_bandpass.hpp"
#include "fir_filter_highpass.hpp"
#include "fir_filter_lowpass.hpp"
#include "plugin_base.hpp"

class Crystalizer : public PluginBase {
 public:
  Crystalizer(const std::string& tag,
              const std::string& schema,
              const std::string& schema_path,
              PipeManager* pipe_manager);
  Crystalizer(const Crystalizer&) = delete;
  auto operator=(const Crystalizer&) -> Crystalizer& = delete;
  Crystalizer(const Crystalizer&&) = delete;
  auto operator=(const Crystalizer&&) -> Crystalizer& = delete;
  ~Crystalizer() override;

  auto get_latency() const -> float;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  sigc::signal<void(double)> new_latency;

 private:
  bool n_samples_is_power_of_2 = true;
  bool filters_are_ready = false;
  bool notify_latency = false;
  bool aggressive = false;

  uint blocksize = 512;
  uint latency_n_frames = 0;

  static constexpr uint nbands = 13;
  static constexpr uint ndivs = 1000;
  static constexpr uint dv = 1.0F / ndivs;

  float latency = 0.0F;

  std::vector<float> data_L;
  std::vector<float> data_R;

  std::array<bool, nbands> band_mute;
  std::array<bool, nbands> band_bypass;

  std::array<float, nbands - 1> frequencies;
  std::array<float, nbands> band_intensity;
  std::array<float, nbands> band_last_L;
  std::array<float, nbands> band_last_R;
  std::array<float, nbands> band_delayed_L;
  std::array<float, nbands> band_delayed_R;

  std::array<std::vector<float>, nbands> band_data_L;
  std::array<std::vector<float>, nbands> band_data_R;
  std::array<std::vector<float>, nbands> band_gain;
  std::array<std::vector<float>, nbands> band_second_derivative_L;
  std::array<std::vector<float>, nbands> band_second_derivative_R;

  std::array<std::unique_ptr<FirFilterBase>, nbands> filters;

  std::deque<float> deque_in_L, deque_in_R;
  std::deque<float> deque_out_L, deque_out_R;

  std::vector<std::jthread> threads;
};

#endif
