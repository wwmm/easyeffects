#include "resampler.hpp"

Resampler::Resampler(const int& input_rate, const int& output_rate) {
  resample_ratio = static_cast<float>(output_rate) / static_cast<float>(input_rate);
}

Resampler::~Resampler() {}