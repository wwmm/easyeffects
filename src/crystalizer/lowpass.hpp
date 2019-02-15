#ifndef LOWPASS_HPP
#define LOWPASS_HPP

#include <zita-convolver.h>
#include "util.hpp"

class Lowpass {
 public:
  Lowpass(const float& fc, const float& tband);

  ~Lowpass();

  bool ready = false;
  uint nsamples;

  Convproc* conv;

  void init_kernel(const float& rate);
  void init_zita(const int& num_samples);
  void process(float* data);

 private:
  std::string log_tag = "lowpass: ";

  uint kernel_size;
  float cutoff, transition_band;
  float* kernel;
};

#endif
