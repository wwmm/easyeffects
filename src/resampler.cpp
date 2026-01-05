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

#include "resampler.hpp"
#include <speex/speex_resampler.h>
#include <format>
#include "util.hpp"

Resampler::Resampler(const int& input_rate, const int& output_rate)
    : resample_ratio(static_cast<double>(output_rate) / static_cast<double>(input_rate)) {
  int err = 0;

  state = speex_resampler_init(1, input_rate, output_rate,
                               SPEEX_RESAMPLER_QUALITY_DESKTOP,  // quality: 0–10
                               &err);

  if (!state || err != RESAMPLER_ERR_SUCCESS) {
    util::warning(std::format("error while initializing speex resampler: {}", speex_resampler_strerror(err)));
  }
}

Resampler::~Resampler() {
  if (state) {
    speex_resampler_destroy(state);
  }
}

void Resampler::set_quality(const int& value) {
  speex_resampler_set_quality(state, value);
}
