#ifndef FILTER_HPP
#define FILTER_HPP

#include <zita-convolver.h>
#include "util.hpp"

class Filter {
 public:
  Filter(const std::string& tag);

  ~Filter();

  bool ready = false;

  void create_lowpass(const int& nsamples, const float& rate, const float& cutoff, const float& transition_band);

  void create_highpass(const int& nsamples, const float& rate, const float& cutoff, const float& transition_band);

  void create_bandpass(const int& nsamples,
                       const float& rate,
                       const float& cutoff1,
                       const float& cutoff2,
                       const float& transition_band);

  void process(float* data);

  void finish();

 private:
  std::string log_tag;

  int kernel_size, nsamples;
  float* kernel = nullptr;

  Convproc* conv = nullptr;

  void create_lowpass_kernel(const float& rate, const float& cutoff, const float& transition_band);

  void create_highpass_kernel(const float& rate, const float& cutoff, const float& transition_band);

  void create_bandpass_kernel(const float& rate,
                              const float& cutoff1,
                              const float& cutoff2,
                              const float& transition_band);

  void init_zita(const int& num_samples);

  void direct_conv(float*& a, float*& b, float*& c, const int& N);
};

#endif
