#ifndef FIR_FILTER_BASE_HPP
#define FIR_FILTER_BASE_HPP

#include <zita-convolver.h>
#include "util.hpp"

class FirFilterBase {
 public:
  FirFilterBase(std::string tag);
  FirFilterBase(const FirFilterBase&) = delete;
  auto operator=(const FirFilterBase&) -> FirFilterBase& = delete;
  FirFilterBase(const FirFilterBase&&) = delete;
  auto operator=(const FirFilterBase&&) -> FirFilterBase& = delete;
  virtual ~FirFilterBase();

 protected:
  std::string log_tag;

  bool kernel_ready = false;
  bool zita_ready = false;
  bool ready = false;

  int kernel_size = 0;
  int n_samples = 0;

  std::vector<float> kernel;

  Convproc* conv = nullptr;

  void setup_zita();

  static void direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c);
};

#endif