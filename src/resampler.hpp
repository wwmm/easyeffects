/**
 * Copyright © 2017-2026 Wellington Wallace
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

#include <speex/speex_resampler.h>
#include <speex/speexdsp_config_types.h>
#include <cmath>
#include <cstddef>
#include <vector>

class Resampler {
 public:
  Resampler(const int& input_rate, const int& output_rate);
  Resampler(const Resampler&) = delete;
  auto operator=(const Resampler&) -> Resampler& = delete;
  Resampler(const Resampler&&) = delete;
  auto operator=(const Resampler&&) -> Resampler& = delete;
  ~Resampler();

  void set_quality(const int& value);

  template <typename T>
  auto process(const T& input) -> const std::vector<float>& {
    // https://deepwiki.com/xiph/speexdsp/2.3-resampler

    const size_t in_frames = input.size();
    const size_t expected_out = std::ceil(in_frames * resample_ratio);

    output.resize(expected_out);

    spx_uint32_t in_len = in_frames;
    spx_uint32_t out_len = expected_out;

    speex_resampler_process_float(state,
                                  0,  // channel index (mono)
                                  input.data(), &in_len, output.data(), &out_len);

    // If fewer samples were produced for any reason, shrink
    output.resize(out_len);

    return output;
  }

 private:
  double resample_ratio = 1.0;

  SpeexResamplerState* state = nullptr;

  std::vector<float> output;
};
