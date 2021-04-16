#include "fir_filter_bandpass.hpp"

FirFilterBandpass::FirFilterBandpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterBandpass::~FirFilterBandpass() = default;

void FirFilterBandpass::setup() {
  auto lowpass_kernel = create_lowpass_kernel(max_frequency, transition_band);

  // high-pass kernel

  auto highpass_kernel = create_lowpass_kernel(min_frequency, transition_band);

  std::ranges::for_each(highpass_kernel, [](auto& v) { v *= -1; });

  highpass_kernel[(highpass_kernel.size() - 1) / 2] += 1;

  // convolving the low-pass and the high-pass kernel to generate the bandpass kernel

  kernel.resize(2 * highpass_kernel.size() - 1);

  direct_conv(lowpass_kernel, highpass_kernel, kernel);

  setup_zita();
}