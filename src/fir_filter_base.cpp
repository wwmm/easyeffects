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

#include "fir_filter_base.hpp"

namespace {

constexpr auto CONVPROC_SCHEDULER_PRIORITY = 0;

constexpr auto CONVPROC_SCHEDULER_CLASS = SCHED_FIFO;

}  // namespace

FirFilterBase::FirFilterBase(std::string tag) : log_tag(std::move(tag)) {}

FirFilterBase::~FirFilterBase() {
  zita_ready = false;

  if (conv != nullptr) {
    conv->stop_process();

    conv->cleanup();

    delete conv;
  }
}

void FirFilterBase::set_rate(const uint& value) {
  rate = value;
}

void FirFilterBase::set_n_samples(const uint& value) {
  n_samples = value;
}

void FirFilterBase::set_min_frequency(const float& value) {
  min_frequency = value;
}

void FirFilterBase::set_max_frequency(const float& value) {
  max_frequency = value;
}

void FirFilterBase::set_transition_band(const float& value) {
  transition_band = value;
}

void FirFilterBase::setup() {}

auto FirFilterBase::create_lowpass_kernel(const float& cutoff, const float& transition_band) const
    -> std::vector<float> {
  std::vector<float> output;

  if (rate == 0) {
    return output;
  }

  /*
    transition band frequency as a fraction of the sample rate
  */

  float b = transition_band / static_cast<float>(rate);

  /*
      The kernel size must be odd: M + 1 where M is even. This is done so it can be symmetric around the main lobe
      https://www.dspguide.com/ch16/1.htm

      The kernel size is related to the transition bandwidth M = 4/BW
  */

  size_t M = std::ceil(4.0F / b);

  M = (M % 2 == 0) ? M : M + 1;  // checking if M is even

  output.resize(M + 1);

  /*
    cutoff frequency as a fraction of the sample rate
  */

  float fc = cutoff / static_cast<float>(rate);

  float sum = 0.0F;

  for (size_t n = 0U, output_size = output.size(); n < output_size; n++) {
    /*
      windowed-sinc kernel https://www.dspguide.com/ch16/1.htm
    */

    if (n == M / 2U) {
      output[n] = 2.0F * std::numbers::pi_v<float> * fc;
    } else {
      output[n] = std::sin(2.0F * std::numbers::pi_v<float> * fc * static_cast<float>(n - static_cast<uint>(M / 2U))) /
                  static_cast<float>(n - static_cast<uint>(M / 2U));
    }

    /*
      Blackman window https://www.dspguide.com/ch16/1.htm
    */

    const auto& w = 0.42F -
                    0.5F * std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) / static_cast<float>(M)) +
                    0.08F * std::cos(4.0F * std::numbers::pi_v<float> * static_cast<float>(n) / static_cast<float>(M));

    output[n] *= w;

    sum += output[n];
  }

  /*
    Normalizing so that we have unit gain at zero frequency
  */

  std::ranges::for_each(output, [&](auto& v) { v /= sum; });

  return output;
}

void FirFilterBase::setup_zita() {
  zita_ready = false;

  if (n_samples == 0U || kernel.empty()) {
    return;
  }

  if (conv == nullptr) {
    conv = new Convproc();
  }

  conv->stop_process();
  conv->cleanup();

  int ret = 0;
  float density = 0.0F;

  conv->set_options(0);

  ret = conv->configure(2, 2, kernel.size(), n_samples, n_samples, n_samples, density);

  if (ret != 0) {
    util::warning(log_tag + "can't initialise zita-convolver engine: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(0, 0, 1, kernel.data(), 0, static_cast<int>(kernel.size()));

  if (ret != 0) {
    util::warning(log_tag + "left impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(1, 1, 1, kernel.data(), 0, static_cast<int>(kernel.size()));

  if (ret != 0) {
    util::warning(log_tag + "right impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    util::warning(log_tag + "start_process failed: " + std::to_string(ret));

    conv->stop_process();
    conv->cleanup();

    return;
  }

  // conv->print();

  zita_ready = true;
}

void FirFilterBase::direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
  uint M = (c.size() + 1U) / 2U;

  for (uint n = 0U; n < c.size(); n++) {
    c[n] = 0.0F;

    for (uint m = 0U; m < M; m++) {
      if (n > m && n - m < M) {
        c[n] += a[n - m] * b[m];
      }
    }
  }
}

auto FirFilterBase::get_delay() const -> float {
  return delay;
}
