#ifndef FILTER_HPP
#define FILTER_HPP

#include <zita-convolver.h>
#include "util.hpp"

enum class Mode { lowpass, highpass };

class Filter {
 public:
  Filter(Mode filter_mode, const float& fc, const float& tband);

  ~Filter();

  bool ready = false;

  void init_kernel(const float& rate);
  void init_zita(const int& num_samples);
  void process(float* data);
  void finish();

 private:
  std::string log_tag = "crystalizer filter: ";

  Mode mode;

  uint kernel_size, nsamples;
  float cutoff, transition_band;
  float* kernel = nullptr;

  Convproc* conv = nullptr;
};

#endif
