#ifndef RESAMPLER_HPP
#define RESAMPLER_HPP

#include <samplerate.h>

class Resampler {
 public:
  Resampler(const int& input_rate, const int& output_rate);
  Resampler(const Resampler&) = delete;
  auto operator=(const Resampler&) -> Resampler& = delete;
  Resampler(const Resampler&&) = delete;
  auto operator=(const Resampler&&) -> Resampler& = delete;
  ~Resampler();

 private:
  float resample_ratio = 1.0F;

  SRC_STATE* src_state = nullptr;
};

#endif