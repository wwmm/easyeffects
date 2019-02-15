#ifndef LOWPASS_HPP
#define LOWPASS_HPP

#include <zita-convolver.h>
#include "util.hpp"

class Lowpass {
 public:
  Lowpass(const int& num_samples, const float& fc);

  ~Lowpass();

 private:
  std::string log_tag = "lowpass: ";

  bool ready = false;
  int kernel_size = 1001, nsamples;
  float* kernel;

  Convproc* conv;

  void init_kernel(const float& fc);
  void init_zita();
};

#endif
