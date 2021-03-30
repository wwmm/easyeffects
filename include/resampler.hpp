#ifndef RESAMPLER_HPP
#define RESAMPLER_HPP

#include <samplerate.h>
#include <span>
#include <vector>

class Resampler {
 public:
  Resampler(const int& input_rate, const int& output_rate);
  Resampler(const Resampler&) = delete;
  auto operator=(const Resampler&) -> Resampler& = delete;
  Resampler(const Resampler&&) = delete;
  auto operator=(const Resampler&&) -> Resampler& = delete;
  ~Resampler();

  auto process(std::span<float>& input) -> std::vector<float>;

 private:
  float resample_ratio = 1.0F;

  SRC_STATE* src_state = nullptr;

  SRC_DATA src_data{};

  std::vector<float> output;
};

#endif