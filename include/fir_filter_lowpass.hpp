#ifndef FIR_FILTER_LOWPASS_HPP
#define FIR_FILTER_LOWPASS_HPP

#include "fir_filter_base.hpp"

class FirFilterLowpass : public FirFilterBase {
 public:
  FirFilterLowpass(std::string tag);
  FirFilterLowpass(const FirFilterLowpass&) = delete;
  auto operator=(const FirFilterLowpass&) -> FirFilterLowpass& = delete;
  FirFilterLowpass(const FirFilterLowpass&&) = delete;
  auto operator=(const FirFilterLowpass&&) -> FirFilterLowpass& = delete;
  ~FirFilterLowpass() override;

  void setup() override;
};

#endif