#include "fir_filter_highpass.hpp"

FirFilterHighpass::FirFilterHighpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterHighpass::~FirFilterHighpass() = default;

void FirFilterHighpass::setup(const uint& rate,
                              const uint& n_samples,
                              const float& cutoff,
                              const float& transition_band) {
  this->rate = rate;
  this->n_samples = n_samples;

  kernel = create_lowpass_kernel(cutoff, transition_band);

  std::ranges::for_each(kernel, [](auto& v) { v *= -1; });

  kernel[(kernel.size() - 1) / 2] += 1;

  setup_zita();
}