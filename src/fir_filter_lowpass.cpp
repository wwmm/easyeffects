#include "fir_filter_lowpass.hpp"

FirFilterLowpass::FirFilterLowpass(std::string tag) : FirFilterBase(std::move(tag)) {}

FirFilterLowpass::~FirFilterLowpass() = default;

void FirFilterLowpass::setup() {
  kernel = create_lowpass_kernel(max_frequency, transition_band);

  setup_zita();
}