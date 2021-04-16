#include "fir_filter_highpass.hpp"

FirFilterHighpass::FirFilterHighpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterHighpass::~FirFilterHighpass() = default;

void FirFilterHighpass::setup() {
  kernel = create_lowpass_kernel(min_frequency, transition_band);

  std::ranges::for_each(kernel, [](auto& v) { v *= -1; });

  kernel[(kernel.size() - 1) / 2] += 1;

  setup_zita();
}