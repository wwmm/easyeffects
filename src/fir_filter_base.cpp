#include "fir_filter_base.hpp"

namespace {

constexpr auto CONVPROC_SCHEDULER_PRIORITY = 0;

constexpr auto CONVPROC_SCHEDULER_CLASS = SCHED_FIFO;

}  // namespace

FirFilterBase::FirFilterBase(std::string tag) : log_tag(std::move(tag)) {}

FirFilterBase::~FirFilterBase() {
  ready = false;
  zita_ready = false;
  kernel_ready = false;

  if (conv != nullptr) {
    if (conv->state() != Convproc::ST_STOP) {
      conv->stop_process();

      conv->cleanup();

      delete conv;
    }
  }
}

void FirFilterBase::setup_zita() {
  zita_ready = false;

  if (n_samples == 0 || !kernel_ready) {
    return;
  }

  conv->stop_process();
  conv->cleanup();

  int ret = 0;
  int max_convolution_size = kernel.size();
  uint options = 0;
  float density = 0.0F;

  options |= Convproc::OPT_VECTOR_MODE;

  conv->set_options(options);

  ret = conv->configure(2, 2, max_convolution_size, n_samples, n_samples, n_samples, density);

  if (ret != 0) {
    util::warning(log_tag + "can't initialise zita-convolver engine: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(0, 0, 1, kernel.data(), 0, kernel.size());

  if (ret != 0) {
    util::debug(log_tag + "left impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->impdata_create(1, 1, 1, kernel.data(), 0, kernel.size());

  if (ret != 0) {
    util::debug(log_tag + "right impdata_create failed: " + std::to_string(ret));

    return;
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    util::debug(log_tag + "start_process failed: " + std::to_string(ret));

    conv->stop_process();
    conv->cleanup();

    return;
  }

  zita_ready = true;

  util::debug(log_tag + "zita is ready");
}

void FirFilterBase::direct_conv(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
  uint M = (c.size() + 1U) / 2U;

  for (uint n = 0U; n < c.size(); n++) {
    c[n] = 0.0F;

    for (uint m = 0U; m < M; m++) {
      if (n > m && n - m < M) {
        c[n] += a[n - m] * b[m];
      }
    }
  }
}