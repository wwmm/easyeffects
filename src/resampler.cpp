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

#include "resampler.hpp"
#include <samplerate.h>

Resampler::Resampler(const int& input_rate, const int& output_rate) : output(1, 0) {
  resample_ratio = static_cast<double>(output_rate) / static_cast<double>(input_rate);

  src_state = src_new(SRC_SINC_FASTEST, 1, nullptr);
}

Resampler::~Resampler() {
  if (src_state != nullptr) {
    src_delete(src_state);
  }
}
