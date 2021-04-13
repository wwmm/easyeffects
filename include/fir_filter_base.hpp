#ifndef FIR_FILTER_BASE_HPP
#define FIR_FILTER_BASE_HPP

#include <zita-convolver.h>
#include <algorithm>
#include <numbers>
#include <ranges>
#include <span>
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

  int n_samples = 0;

  std::vector<float> kernel;

  Convproc* conv = nullptr;

  void create_lowpass_kernel(const float& rate, const float& cutoff, const float& transition_band);

  void setup_zita();

  static void direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c);

  template <typename T1>
  void process(T1& data_left, T1& data_right) {
    std::span conv_left_in{conv->inpdata(0), conv->inpdata(0) + n_samples};
    std::span conv_right_in{conv->inpdata(1), conv->inpdata(1) + n_samples};

    std::span conv_left_out{conv->outdata(0), conv->outdata(0) + n_samples};
    std::span conv_right_out{conv->outdata(1), conv->outdata(1) + n_samples};

    std::copy(data_left.begin(), data_left.end(), conv_left_in.begin());
    std::copy(data_right.begin(), data_right.end(), conv_right_in.begin());

    if (conv->state() == Convproc::ST_PROC) {
      int ret = conv->process(true);  // thread sync mode set to true

      if (ret != 0) {
        util::debug(log_tag + "IR: process failed: " + std::to_string(ret));

        zita_ready = false;
      } else {
        std::copy(conv_left_out.begin(), conv_left_out.end(), data_left.begin());
        std::copy(conv_right_out.begin(), conv_right_out.end(), data_right.begin());
      }
    }
  }
};

#endif