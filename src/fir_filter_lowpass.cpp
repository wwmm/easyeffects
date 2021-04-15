#include "fir_filter_lowpass.hpp"

FirFilterLowpass::FirFilterLowpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterLowpass::~FirFilterLowpass() = default;

void FirFilterLowpass::setup(const uint& rate,
                             const uint& n_samples,
                             const float& cutoff,
                             const float& transition_band) {
  this->rate = rate;
  this->n_samples = n_samples;

  kernel_ready = false;

  create_lowpass_kernel(cutoff, transition_band);

  kernel_ready = true;
}