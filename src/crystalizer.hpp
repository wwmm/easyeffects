/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include <qtmetamacros.h>
#include <sys/types.h>
#include <QString>
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_crystalizer.h"
#include "fir_filter_bandpass.hpp"
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

  std::array<std::vector<float>, nbands> band_data_L;
  std::array<std::vector<float>, nbands> band_data_R;
  std::array<std::vector<float>, nbands> band_gain;
  std::array<std::vector<float>, nbands> band_second_derivative_L;
  std::array<std::vector<float>, nbands> band_second_derivative_R;

  std::array<std::unique_ptr<FirFilterBase>, nbands> filters;

  std::deque<float> deque_out_L, deque_out_R;

  template <typename T1>
  void enhance_peaks(T1& data_left, T1& data_right) {
    for (uint n = 0U; n < nbands; n++) {
      std::copy(data_left.begin(), data_left.end(), band_data_L.at(n).begin());
      std::copy(data_right.begin(), data_right.end(), band_data_R.at(n).begin());

      filters.at(n)->process(band_data_L.at(n), band_data_R.at(n));

      /*
        Later we will need to calculate the second derivative of each band. This
        is done through the central difference method. In order to calculate
        the derivative at the last elements of the array we have to know the first
        element of the next buffer. As we do not have this information we will assume for simplicity that the last
        value of the current buffer is a good enough approximation for the first element of the next buffer
      */

      band_next_L.at(n) = band_data_L.at(n)[blocksize - 1U];
      band_next_R.at(n) = band_data_R.at(n)[blocksize - 1U];

      if (is_first_buffer) {
        band_previous_L.at(n) = band_data_L.at(n)[0];
        band_previous_R.at(n) = band_data_R.at(n)[0];

        is_first_buffer = false;
      }
    }

    for (uint n = 0U; n < nbands; n++) {
      // Calculating the second derivative

      if (!band_bypass.at(n)) {
        for (uint m = 0U; m < blocksize; m++) {
          const float L = band_data_L.at(n)[m];
          const float R = band_data_R.at(n)[m];

          if (m > 0U && m < blocksize - 1U) {
            const float& L_lower = band_data_L.at(n)[m - 1U];
            const float& R_lower = band_data_R.at(n)[m - 1U];
            const float& L_upper = band_data_L.at(n)[m + 1U];
            const float& R_upper = band_data_R.at(n)[m + 1U];

            band_second_derivative_L.at(n)[m] = L_upper - 2.0F * L + L_lower;
            band_second_derivative_R.at(n)[m] = R_upper - 2.0F * R + R_lower;
          } else if (m == 0U) {
            const float& L_lower = band_previous_L.at(n);
            const float& R_lower = band_previous_R.at(n);
            const float& L_upper = band_data_L.at(n)[m + 1U];
            const float& R_upper = band_data_R.at(n)[m + 1U];

            band_second_derivative_L.at(n)[m] = L_upper - 2.0F * L + L_lower;
            band_second_derivative_R.at(n)[m] = R_upper - 2.0F * R + R_lower;
          } else if (m == blocksize - 1U) {
            const float& L_upper = band_next_L.at(n);
            const float& R_upper = band_next_R.at(n);
            const float& L_lower = band_data_L.at(n)[m - 1U];
            const float& R_lower = band_data_R.at(n)[m - 1U];

            band_second_derivative_L.at(n)[m] = L_upper - 2.0F * L + L_lower;
            band_second_derivative_R.at(n)[m] = R_upper - 2.0F * R + R_lower;
          }
        }

        // peak enhancing using second derivative

        for (uint m = 0U; m < blocksize; m++) {
          const float L = band_data_L.at(n)[m];
          const float R = band_data_R.at(n)[m];
          const float& d2L = band_second_derivative_L.at(n)[m];
          const float& d2R = band_second_derivative_R.at(n)[m];

          /*
            The correct approach would be to avoid the second derivative getting too big... But using tanh to smoothly
            staying between [-1, 1] seems to be enough
          */

          band_data_L.at(n)[m] = L - std::tanh(band_intensity.at(n) * d2L);
          band_data_R.at(n)[m] = R - std::tanh(band_intensity.at(n) * d2R);

          if (m == blocksize - 1U) {
            band_previous_L.at(n) = L;
            band_previous_R.at(n) = R;
          }
        }
      } else {
        band_previous_L.at(n) = band_data_L.at(n)[blocksize - 1U];
        band_previous_R.at(n) = band_data_R.at(n)[blocksize - 1U];
      }
    }

    // add bands

    for (uint m = 0U; m < blocksize; m++) {
      data_left[m] = 0.0F;
      data_right[m] = 0.0F;

      for (uint n = 0U; n < nbands; n++) {
        if (!band_mute.at(n)) {
          data_left[m] += band_data_L.at(n)[m];
          data_right[m] += band_data_R.at(n)[m];
        }
      }
    }
  }
};
