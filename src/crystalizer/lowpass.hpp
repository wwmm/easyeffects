#ifndef LOWPASS_HPP
#define LOWPASS_HPP

#include <zita-convolver.h>
#include "util.hpp"

class Lowpass {
 public:
  Lowpass(const float& fc);

  ~Lowpass();

  bool ready = false;
  uint nsamples;

  Convproc* conv;

  void init_kernel(const float& fc);
  void init_zita(const int& num_samples);
  void process(float* data);

 private:
  std::string log_tag = "lowpass: ";

  uint kernel_size = 10001;
  float* kernel;
};

#endif
