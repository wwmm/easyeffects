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

#include "fir_filter_lowpass.hpp"

FirFilterLowpass::FirFilterLowpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterLowpass::~FirFilterLowpass() = default;

void FirFilterLowpass::setup() {
  auto tmp = create_lowpass_kernel(max_frequency, transition_band);

  /*
    Convolving the kernel with itself to increase the stopband attenuation https://www.dspguide.com/ch16/4.htm
  */

  kernel.resize(2 * tmp.size() - 1);

  direct_conv(tmp, tmp, kernel);

  delay = 0.5F * static_cast<float>(kernel.size() - 1) / static_cast<float>(rate);

  setup_zita();
}
