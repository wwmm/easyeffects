#include "resampler.hpp"

Resampler::Resampler(const int& input_rate, const int& output_rate) : output(1, 0) {
  resample_ratio = static_cast<float>(output_rate) / static_cast<float>(input_rate);

  src_state = src_new(SRC_SINC_FASTEST, 1, nullptr);
}

Resampler::~Resampler() {
  if (src_state != nullptr) {
    src_delete(src_state);
  }
}
