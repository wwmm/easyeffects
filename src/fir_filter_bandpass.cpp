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

#include "fir_filter_bandpass.hpp"

FirFilterBandpass::FirFilterBandpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterBandpass::~FirFilterBandpass() = default;

void FirFilterBandpass::setup() {
  auto lowpass_kernel = create_lowpass_kernel(max_frequency, transition_band);

  // high-pass kernel

  auto highpass_kernel = create_lowpass_kernel(min_frequency, transition_band);

  std::ranges::for_each(highpass_kernel, [](auto& v) { v *= -1; });

  highpass_kernel[(highpass_kernel.size() - 1) / 2] += 1;

  std::vector<float> tmp(highpass_kernel.size());

  /*
    Creating a bandpass from a band reject through spectral inversion https://www.dspguide.com/ch16/4.htm
  */

  for (size_t n = 0; n < tmp.size(); n++) {
    tmp[n] = lowpass_kernel[n] + highpass_kernel[n];
  }

  std::ranges::for_each(tmp, [](auto& v) { v *= -1; });

  tmp[(tmp.size() - 1) / 2] += 1;

  /*
    Convolving the kernel with itself to increase the stopband attenuation https://www.dspguide.com/ch16/4.htm
  */

  kernel.resize(2 * tmp.size() - 1);

  direct_conv(tmp, tmp, kernel);

  delay = 0.5F * static_cast<float>(kernel.size() - 1) / static_cast<float>(rate);

  setup_zita();
}
