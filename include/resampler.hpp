#ifndef RESAMPLER_HPP
#define RESAMPLER_HPP

#include <samplerate.h>
#include <cmath>
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

  template <typename T>
  auto process(const T& input, const bool& end_of_input) -> std::vector<float> {
    output.resize(std::ceil(resample_ratio * input.size()));

    // The number of frames of data pointed to by data_in
    src_data.input_frames = input.size();

    // A pointer to the input data samples
    src_data.data_in = input.data();

    // Maximum number of frames pointed to by data_out
    src_data.output_frames = output.size();

    // A pointer to the output data samples
    src_data.data_out = output.data();

    // Equal to output_sample_rate / input_sample_rate
    src_data.src_ratio = resample_ratio;

    // Equal to 0 if more input data is available and 1 otherwise
    src_data.end_of_input = static_cast<int>(end_of_input);

    src_process(src_state, &src_data);

    output.resize(src_data.output_frames_gen);

    return output;
  }

 private:
  float resample_ratio = 1.0F;

  SRC_STATE* src_state = nullptr;

  SRC_DATA src_data{};

  std::vector<float> output;
};

#endif