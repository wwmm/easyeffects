#include "resampler.hpp"

Resampler::Resampler(const int& input_rate, const int& output_rate) {
  resample_ratio = static_cast<float>(output_rate) / static_cast<float>(input_rate);

  src_state = src_new(SRC_SINC_BEST_QUALITY, 1, nullptr);
}

Resampler::~Resampler() {
  if (src_state != nullptr) {
    src_delete(src_state);
  }
}

auto Resampler::process(std::span<float>& input) -> std::vector<float> {
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
  src_data.end_of_input = 0;

  src_process(src_state, &src_data);

  return output;
}