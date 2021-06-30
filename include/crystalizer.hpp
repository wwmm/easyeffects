/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CRYSTALIZER_HPP
#define CRYSTALIZER_HPP

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

  sigc::signal<void(double)> latency;

 private:
  bool n_samples_is_power_of_2 = true;
  bool filters_are_ready = false;
  bool notify_latency = false;
  bool do_first_rotation = true;

  uint blocksize = 512;
  uint latency_n_frames = 0;

  static constexpr uint nbands = 13;

  std::vector<float> data_L;
  std::vector<float> data_R;

  std::array<bool, nbands> band_mute;
  std::array<bool, nbands> band_bypass;

  std::array<float, nbands - 1> frequencies;
  std::array<float, nbands> band_intensity;
  std::array<float, nbands> band_last_L;
  std::array<float, nbands> band_last_R;
  std::array<float, nbands> band_next_L;
  std::array<float, nbands> band_next_R;

  std::array<std::vector<float>, nbands> band_data_L;
  std::array<std::vector<float>, nbands> band_data_R;
  std::array<std::vector<float>, nbands> band_gain;
  std::array<std::vector<float>, nbands> band_second_derivative_L;
  std::array<std::vector<float>, nbands> band_second_derivative_R;

  std::array<std::unique_ptr<FirFilterBase>, nbands> filters;

  std::deque<float> deque_out_L, deque_out_R;

  void bind_band(const int& n);

  template <typename T1>
  void enhance_peaks(T1& data_left, T1& data_right) {
    for (uint n = 0; n < nbands; n++) {
      std::copy(data_left.begin(), data_left.end(), band_data_L.at(n).begin());
      std::copy(data_right.begin(), data_right.end(), band_data_R.at(n).begin());

      filters.at(n)->process(band_data_L.at(n), band_data_R.at(n));

      /*
      Later we will need to calculate the second derivative of each band. This
      is done through the central difference method. In order to calculate
      the derivative at the last elements of the array we have to know the first
      element of the next buffer. As we do not have this information the only
      way to do this calculation is delaying the signal by 1 sample.
    */

      // last (R,L) becomes the first

      std::rotate(band_data_L.at(n).rbegin(), band_data_L.at(n).rbegin() + 1, band_data_L.at(n).rend());
      std::rotate(band_data_R.at(n).rbegin(), band_data_R.at(n).rbegin() + 1, band_data_R.at(n).rend());

      if (do_first_rotation) {
        /*
          band_data was rotated. Its first values are the last ones from the original array. we have to save them for
          the next round.
        */

        band_next_L.at(n) = band_data_L.at(n)[0];
        band_next_R.at(n) = band_data_R.at(n)[0];

        band_last_L.at(n) = 0.0F;
        band_last_R.at(n) = 0.0F;

        band_data_L.at(n)[0] = 0.0F;
        band_data_R.at(n)[0] = 0.0F;

        do_first_rotation = false;
      } else {
        /*
          band_data was rotated. Its first values are the last ones from the original array. we have to save them for
          the next round.
        */

        float L = band_data_L.at(n)[0];
        float R = band_data_R.at(n)[0];

        band_data_L.at(n)[0] = band_next_L.at(n);
        band_data_R.at(n)[0] = band_next_R.at(n);

        band_next_L.at(n) = L;
        band_next_R.at(n) = R;
      }
    }

    for (uint n = 0; n < nbands; n++) {
      // Calculating the second derivative

      if (!band_bypass.at(n)) {
        for (uint m = 0; m < blocksize; m++) {
          float L = band_data_L.at(n)[m];
          float R = band_data_R.at(n)[m];

          if (m > 0 && m < blocksize - 1) {
            float L_lower = band_data_L.at(n)[m - 1];
            float R_lower = band_data_R.at(n)[m - 1];
            float L_upper = band_data_L.at(n)[m + 1];
            float R_upper = band_data_R.at(n)[m + 1];

            band_second_derivative_L.at(n)[m] = L_upper - 2.0F * L + L_lower;
            band_second_derivative_R.at(n)[m] = R_upper - 2.0F * R + R_lower;
          } else if (m == 0U) {
            float L_lower = band_last_L.at(n);
            float R_lower = band_last_R.at(n);
            float L_upper = band_data_L.at(n)[m + 1];
            float R_upper = band_data_R.at(n)[m + 1];

            band_second_derivative_L.at(n)[m] = L_upper - 2.0F * L + L_lower;
            band_second_derivative_R.at(n)[m] = R_upper - 2.0F * R + R_lower;
          } else if (m == blocksize - 1) {
            float L_upper = band_next_L.at(n);
            float R_upper = band_next_R.at(n);
            float L_lower = band_data_L.at(n)[m - 1];
            float R_lower = band_data_R.at(n)[m - 1];

            band_second_derivative_L.at(n)[m] = L_upper - 2.0F * L + L_lower;
            band_second_derivative_R.at(n)[m] = R_upper - 2.0F * R + R_lower;
          }
        }

        // peak enhancing using second derivative

        for (uint m = 0; m < blocksize; m++) {
          float L = band_data_L.at(n)[m];
          float R = band_data_R.at(n)[m];
          float d2L = band_second_derivative_L.at(n)[m];
          float d2R = band_second_derivative_R.at(n)[m];

          band_data_L.at(n)[m] = L - band_intensity.at(n) * d2L;
          band_data_R.at(n)[m] = R - band_intensity.at(n) * d2R;

          if (m == blocksize - 1) {
            band_last_L.at(n) = L;
            band_last_R.at(n) = R;
          }
        }
      } else {
        band_last_L.at(n) = band_data_L.at(n)[blocksize - 1];
        band_last_R.at(n) = band_data_R.at(n)[blocksize - 1];
      }
    }

    // add bands

    for (uint m = 0; m < blocksize; m++) {
      data_left[m] = 0.0F;
      data_right[m] = 0.0F;

      for (uint n = 0; n < nbands; n++) {
        if (!band_mute.at(n)) {
          data_left[m] += band_data_L.at(n)[m];
          data_right[m] += band_data_R.at(n)[m];
        }
      }
    }
  }
};

#endif
