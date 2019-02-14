#ifndef LOWPASS_HPP
#define LOWPASS_HPP

#include <zita-convolver.h>
#include "util.hpp"

class Lowpass {
 public:
  Lowpass(const int& num_samples);

  ~Lowpass();

 private:
  std::string log_tag = "lowpass: ";

  bool ready = false;
  int kernel_size = 1001, nsamples;
  float *kernel_L, *kernel_R;  // left and right channels buffers

  Convproc* conv;
};

#endif
