/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include <qtmetamacros.h>
#include <sys/types.h>
#include <QString>
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <memory>
#include <numbers>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_crystalizer.h"
#include "fir_filter_base.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Crystalizer : public PluginBase {
  Q_OBJECT

 public:
  Crystalizer(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Crystalizer(const Crystalizer&) = delete;
  auto operator=(const Crystalizer&) -> Crystalizer& = delete;
  Crystalizer(const Crystalizer&&) = delete;
  auto operator=(const Crystalizer&&) -> Crystalizer& = delete;
  ~Crystalizer() override;

  void reset() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  auto get_latency_seconds() -> float override;

 private:
  bool n_samples_is_power_of_2 = true;
  bool filters_are_ready = false;
  bool notify_latency = false;
  bool is_first_buffer = true;

  uint blocksize = 512U;
  uint latency_n_frames = 0U;

  static constexpr uint nbands = 13U;

  float block_time = 0.0;
  float attack_time = 0.1;   // seconds
  float release_time = 0.4;  // seconds

  db::Crystalizer* settings = nullptr;

  std::vector<float> data_L;
  std::vector<float> data_R;

  std::array<bool, nbands> band_mute;
  std::array<bool, nbands> band_bypass;

  std::array<float, nbands + 1U> frequencies;
  std::array<float, nbands> band_intensity;
  std::array<float, nbands> band_previous_L;
  std::array<float, nbands> band_previous_R;
  std::array<float, nbands> band_next_L;
  std::array<float, nbands> band_next_R;

  std::array<float, nbands> band_prev_intensity_L;
  std::array<float, nbands> band_prev_intensity_R;

  std::array<std::vector<float>, nbands> band_data_L;
  std::array<std::vector<float>, nbands> band_data_R;
  std::array<std::vector<float>, nbands> band_gain;
  std::array<std::vector<float>, nbands> band_second_derivative_L;
  std::array<std::vector<float>, nbands> band_second_derivative_R;

  std::array<std::unique_ptr<FirFilterBase>, nbands> filters;

  std::deque<float> deque_out_L, deque_out_R;

  float compute_adaptive_intensity(float base_intensity, float* band_data) const;

  template <typename T1>
  void enhance_peaks(T1& data_left, T1& data_right) {
    for (uint n = 0U; n < nbands; n++) {
      auto& bandn_L = band_data_L.at(n);
      auto& bandn_R = band_data_R.at(n);

      std::copy(data_left.begin(), data_left.end(), bandn_L.begin());
      std::copy(data_right.begin(), data_right.end(), bandn_R.begin());

      filters.at(n)->process(bandn_L, bandn_R);

      /**
       * Later we will need to calculate the second derivative of each band.
       * This is done through the central difference method. In order to
       * calculate the derivative at the last elements of the array we have to
       * know the first element of the next buffer.
       * As we do not have this information we will assume for simplicity that
       * the last value of the current buffer is a good enough approximation
       * for the first element of the next buffer
       */

      band_next_L.at(n) = bandn_L[blocksize - 1U];
      band_next_R.at(n) = bandn_R[blocksize - 1U];

      if (is_first_buffer) {
        band_previous_L.at(n) = bandn_L[0];
        band_previous_R.at(n) = bandn_R[0];

        is_first_buffer = false;
      }
    }

    for (uint n = 0U; n < nbands; n++) {
      // Calculating the second derivative

      auto bandn_L = band_data_L.at(n).data();
      auto bandn_R = band_data_R.at(n).data();

      auto& prev_intensity_L = band_prev_intensity_L.at(n);
      auto& prev_intensity_R = band_prev_intensity_R.at(n);

      if (!band_bypass.at(n)) {
        const float intensity = band_intensity.at(n);
        float intensity_L = intensity;
        float intensity_R = intensity;

        if (settings->adaptiveIntensity()) {
          intensity_L = compute_adaptive_intensity(intensity, bandn_L);
          intensity_R = compute_adaptive_intensity(intensity, bandn_R);

          // leaky integrator

          auto tau_L = (prev_intensity_L < intensity_L) ? attack_time : release_time;
          auto tau_R = (prev_intensity_R < intensity_R) ? attack_time : release_time;

          float alpha_L = std::exp(-block_time / tau_L);
          float alpha_R = std::exp(-block_time / tau_R);

          intensity_L = (alpha_L * prev_intensity_L) + ((1.0 - alpha_L) * intensity_L);
          intensity_R = (alpha_R * prev_intensity_R) + ((1.0 - alpha_R) * intensity_R);

          prev_intensity_L = intensity_L;
          prev_intensity_R = intensity_R;

          // util::warning(std::format("band = {} L = {}, R = {}", n, intensity_L, intensity_R));
        }

        auto bandn_second_derivative_L = band_second_derivative_L.at(n).data();
        auto bandn_second_derivative_R = band_second_derivative_R.at(n).data();

        bandn_second_derivative_L[0] = bandn_L[1U] - 2.0F * bandn_L[0U] + band_previous_L.at(n);
        bandn_second_derivative_R[0] = bandn_R[1U] - 2.0F * bandn_R[0U] + band_previous_R.at(n);

        for (uint m = 1U; m < blocksize - 1U; m++) {
          bandn_second_derivative_L[m] = bandn_L[m + 1U] - 2.0F * bandn_L[m] + bandn_L[m - 1U];
          bandn_second_derivative_R[m] = bandn_R[m + 1U] - 2.0F * bandn_R[m] + bandn_R[m - 1U];
        }

        bandn_second_derivative_L[blocksize - 1] =
            band_next_L.at(n) - 2.0F * bandn_L[blocksize - 1] + bandn_L[blocksize - 2];
        bandn_second_derivative_R[blocksize - 1] =
            band_next_R.at(n) - 2.0F * bandn_R[blocksize - 1] + bandn_R[blocksize - 2];

        // peak enhancing using second derivative

        for (uint m = 0U; m < blocksize; m++) {
          const float& d2L = bandn_second_derivative_L[m];
          const float& d2R = bandn_second_derivative_R[m];

          bandn_L[m] = bandn_L[m] - intensity_L * d2L;
          bandn_R[m] = bandn_R[m] - intensity_R * d2R;
        }

        band_previous_L.at(n) = bandn_L[blocksize - 1U];
        band_previous_R.at(n) = bandn_R[blocksize - 1U];
      } else {
        band_previous_L.at(n) = bandn_L[blocksize - 1U];
        band_previous_R.at(n) = bandn_R[blocksize - 1U];
      }
    }

    // add bands

    std::ranges::fill(data_left, 0.0F);
    std::ranges::fill(data_right, 0.0F);

    auto data_left_ptr = data_left.data();
    auto data_right_ptr = data_right.data();

    for (uint n = 0U; n < nbands; n++) {
      if (!band_mute.at(n)) {
        auto bandn_L = band_data_L.at(n).data();
        auto bandn_R = band_data_R.at(n).data();

        for (uint m = 0U; m < blocksize; m++) {
          data_left_ptr[m] += bandn_L[m];
          data_right_ptr[m] += bandn_R[m];
        }
      }
    }

    /**
     * The correct approach would be to avoid the second derivative
     * getting too big... But using tanh to smoothly staying between
     * [-1, 1] seems to be enough
     */

    for (uint m = 0U; m < blocksize; m++) {
      data_left_ptr[m] = std::tanh(data_left_ptr[m]);
      data_right_ptr[m] = std::tanh(data_right_ptr[m]);
    }
  }
};
