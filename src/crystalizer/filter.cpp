#include "filter.hpp"
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/sinc.hpp>

#define CONVPROC_SCHEDULER_PRIORITY 0
#define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
#define THREAD_SYNC_MODE true

const float PI = boost::math::constants::pi<float>();

Filter::Filter(Mode filter_mode, const float& fc, const float& tband)
    : mode(filter_mode), cutoff(fc), transition_band(tband) {}

Filter::~Filter() {
  util::warning(log_tag + "destructed");

  finish();
}

void Filter::init_kernel(const float& rate) {
  float b = transition_band / rate;

  kernel_size = std::ceil(4.0f / b);

  kernel_size = (kernel_size % 2 == 0) ? kernel_size + 1 : kernel_size;

  float fc = cutoff / rate;

  util::debug(log_tag + "kernel size = " + std::to_string(kernel_size));
  // util::debug(log_tag + "fc = " + std::to_string(fc));
  // util::debug(log_tag + "b = " + std::to_string(b));

  kernel = new float[kernel_size];

  float sum = 0.0f;

  for (uint n = 0; n < kernel_size; n++) {
    kernel[n] = boost::math::sinc_pi(2.0f * fc * PI *
                                     (n - (kernel_size - 1.0f) / 2.0f));

    auto w = 0.42f - 0.5f * cosf(2.0f * PI * n / (kernel_size - 1)) +
             0.08f * cosf(4.0f * PI * n / (kernel_size - 1));

    kernel[n] *= w;

    sum += kernel[n];
  }

  if (sum > 0.0f) {
    for (uint n = 0; n < kernel_size; n++) {
      kernel[n] /= sum;
    }
  }

  if (mode == Mode::highpass) {
    for (uint n = 0; n < kernel_size; n++) {
      kernel[n] *= -1;
    }

    kernel[(kernel_size - 1) / 2] += 1;
  }
}

void Filter::init_zita(const int& num_samples) {
  bool failed = false;
  float density = 0.0f;
  int ret;
  unsigned int options = 0;

  nsamples = num_samples;

  // options |= Convproc::OPT_FFTW_MEASURE;
  options |= Convproc::OPT_VECTOR_MODE;

  conv = new Convproc();

  conv->set_options(options);

#if ZITA_CONVOLVER_MAJOR_VERSION == 3
  conv->set_density(density);

  ret =
      conv->configure(2, 2, kernel_size, nsamples, nsamples, Convproc::MAXPART);
#endif

#if ZITA_CONVOLVER_MAJOR_VERSION == 4
  ret = conv->configure(2, 2, kernel_size, nsamples, nsamples,
                        Convproc::MAXPART, density);
#endif

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "can't initialise zita-convolver engine: " +
                std::to_string(ret));
  }

  ret = conv->impdata_create(0, 0, 1, kernel, 0, kernel_size);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "left impdata_create failed: " + std::to_string(ret));
  }

  ret = conv->impdata_create(1, 1, 1, kernel, 0, kernel_size);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag +
                "right impdata_create failed: " + std::to_string(ret));
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY,
                            CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "start_process failed: " + std::to_string(ret));
  }

  if (failed) {
    ready = false;
  } else {
    ready = true;
  }
}

void Filter::process(float* data) {
  if (ready) {
    // deinterleave
    for (uint n = 0; n < nsamples; n++) {
      conv->inpdata(0)[n] = data[2 * n];
      conv->inpdata(1)[n] = data[2 * n + 1];
    }

    int ret = conv->process(THREAD_SYNC_MODE);

    if (ret != 0) {
      util::debug(log_tag + "IR: process failed: " + std::to_string(ret));
    }

    // interleave
    for (uint n = 0; n < nsamples; n++) {
      data[2 * n] = conv->outdata(0)[n];
      data[2 * n + 1] = conv->outdata(1)[n];
    }
  }
}

void Filter::finish() {
  ready = false;

  if (conv != nullptr) {
    if (conv->state() != Convproc::ST_STOP) {
      conv->stop_process();

      conv->cleanup();

      delete conv;

      conv = nullptr;
    }
  }

  if (kernel != nullptr) {
    delete[] kernel;

    kernel = nullptr;
  }
}
