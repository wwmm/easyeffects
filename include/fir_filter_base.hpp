/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#ifndef FIR_FILTER_BASE_HPP
#define FIR_FILTER_BASE_HPP

#include <zita-convolver.h>
#include <algorithm>
#include <numbers>
#include <ranges>
#include <span>
#include "util.hpp"

class FirFilterBase {
 public:
  FirFilterBase(std::string tag);
  FirFilterBase(const FirFilterBase&) = delete;
  auto operator=(const FirFilterBase&) -> FirFilterBase& = delete;
  FirFilterBase(const FirFilterBase&&) = delete;
  auto operator=(const FirFilterBase&&) -> FirFilterBase& = delete;
  virtual ~FirFilterBase();

  void set_rate(const uint& value);

  void set_n_samples(const uint& value);

  void set_min_frequency(const float& value);

  void set_max_frequency(const float& value);

  void set_transition_band(const float& value);

  virtual void setup();

  [[nodiscard]] auto get_delay() const -> float;

  template <typename T1>
  void process(T1& data_left, T1& data_right) {
    std::span conv_left_in{conv->inpdata(0), conv->inpdata(0) + n_samples};
    std::span conv_right_in{conv->inpdata(1), conv->inpdata(1) + n_samples};

    std::span conv_left_out{conv->outdata(0), conv->outdata(0) + n_samples};
    std::span conv_right_out{conv->outdata(1), conv->outdata(1) + n_samples};

    std::copy(data_left.begin(), data_left.end(), conv_left_in.begin());
    std::copy(data_right.begin(), data_right.end(), conv_right_in.begin());

    if (zita_ready) {
      const int& ret = conv->process(true);  // thread sync mode set to true

      if (ret != 0) {
        util::debug(log_tag + "IR: process failed: " + std::to_string(ret));

        zita_ready = false;
      } else {
        std::copy(conv_left_out.begin(), conv_left_out.end(), data_left.begin());
        std::copy(conv_right_out.begin(), conv_right_out.end(), data_right.begin());
      }
    }
  }

 protected:
  const std::string log_tag;

  bool zita_ready = false;

  uint n_samples = 0U;
  uint rate = 0U;

  float min_frequency = 20.0F;
  float max_frequency = 22000.0F;
  float transition_band = 100.0F;  // Hz
  float delay = 0.0F;

  std::vector<float> kernel;

  Convproc* conv = nullptr;

  [[nodiscard]] auto create_lowpass_kernel(const float& cutoff, const float& transition_band) const
      -> std::vector<float>;

  void setup_zita();

  static void direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c);
};

#endif
