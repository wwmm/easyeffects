#include "filter.hpp"
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/sinc.hpp>

#define CONVPROC_SCHEDULER_PRIORITY 0
#define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
#define THREAD_SYNC_MODE true

const float PI = boost::math::constants::pi<float>();

Filter::Filter(const std::string& tag) : log_tag(tag) {}

Filter::~Filter() {
  util::warning(log_tag + " destructed");

  finish();
}

void Filter::create_lowpass_kernel(const float& rate, const float& cutoff, const float& transition_band) {
  float b = transition_band / rate;

  kernel_size = std::ceil(4.0f / b);

  kernel_size = (kernel_size % 2 == 0) ? kernel_size + 1 : kernel_size;

  float fc = cutoff / rate;

  if (kernel != nullptr) {
    delete[] kernel;

    kernel = nullptr;
  }

  kernel = new float[kernel_size];

  float sum = 0.0f;

  for (int n = 0; n < kernel_size; n++) {
    kernel[n] = boost::math::sinc_pi(2.0f * fc * PI * (n - (kernel_size - 1) / 2));

    auto w = 0.42f - 0.5f * cosf(2.0f * PI * n / (kernel_size - 1)) + 0.08f * cosf(4.0f * PI * n / (kernel_size - 1));

    kernel[n] *= w;

    sum += kernel[n];
  }

  for (int n = 0; n < kernel_size; n++) {
    kernel[n] /= sum;
  }
}

void Filter::create_highpass_kernel(const float& rate, const float& cutoff, const float& transition_band) {
  create_lowpass_kernel(rate, cutoff, transition_band);

  for (int n = 0; n < kernel_size; n++) {
    kernel[n] *= -1;
  }

  kernel[(kernel_size - 1) / 2] += 1;
}

void Filter::create_bandpass_kernel(const float& rate,
                                    const float& cutoff1,
                                    const float& cutoff2,
                                    const float& transition_band) {
  create_lowpass_kernel(rate, cutoff2, transition_band);

  float* lowpass_kernel = new float[kernel_size];

  memcpy(lowpass_kernel, kernel, kernel_size * sizeof(float));

  create_highpass_kernel(rate, cutoff1, transition_band);

  float* highpass_kernel = new float[kernel_size];

  memcpy(highpass_kernel, kernel, kernel_size * sizeof(float));

  delete[] kernel;

  kernel_size = 2 * kernel_size - 1;

  kernel = new float[kernel_size];

  direct_conv(lowpass_kernel, highpass_kernel, kernel, kernel_size);

  delete[] lowpass_kernel;
  delete[] highpass_kernel;
}

void Filter::direct_conv(float*& a, float*& b, float*& c, const int& N) {
  int M = (N + 1) / 2;

  for (int n = 0; n < N; n++) {
    c[n] = 0.0f;

    for (int m = 0; m < M; m++) {
      if (n > m && n - m < M) {
        c[n] += a[n - m] * b[m];
      }
    }
  }
}

void Filter::create_lowpass(const int& nsamples, const float& rate, const float& cutoff, const float& transition_band) {
  create_lowpass_kernel(rate, cutoff, transition_band);

  // util::debug(log_tag + " kernel size = " + std::to_string(kernel_size));

  init_zita(nsamples);
}

void Filter::create_highpass(const int& nsamples,
                             const float& rate,
                             const float& cutoff,
                             const float& transition_band) {
  create_highpass_kernel(rate, cutoff, transition_band);

  // util::debug(log_tag + " kernel size = " + std::to_string(kernel_size));

  init_zita(nsamples);
}

void Filter::create_bandpass(const int& nsamples,
                             const float& rate,
                             const float& cutoff1,
                             const float& cutoff2,
                             const float& transition_band) {
  create_bandpass_kernel(rate, cutoff1, cutoff2, transition_band);

  // util::debug(log_tag + " kernel size = " + std::to_string(kernel_size));

  init_zita(nsamples);
}

void Filter::init_zita(const int& num_samples) {
  bool failed = false;
  float density = 0.0f;
  int ret;
  unsigned int options = 0;

  nsamples = num_samples;

  // depending on buffer and kernel size OPT_FFTW_MEASURE may make un crash
  // options |= Convproc::OPT_FFTW_MEASURE;
  options |= Convproc::OPT_VECTOR_MODE;

  conv = new Convproc();

  conv->set_options(options);

#if ZITA_CONVOLVER_MAJOR_VERSION == 3
  conv->set_density(density);

  ret = conv->configure(2, 2, kernel_size, nsamples, nsamples, Convproc::MAXPART);
#endif

#if ZITA_CONVOLVER_MAJOR_VERSION == 4
  ret = conv->configure(2, 2, kernel_size, nsamples, nsamples, Convproc::MAXPART, density);
#endif

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "can't initialise zita-convolver engine: " + std::to_string(ret));
  }

  ret = conv->impdata_create(0, 0, 1, kernel, 0, kernel_size);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "left impdata_create failed: " + std::to_string(ret));
  }

  ret = conv->impdata_create(1, 1, 1, kernel, 0, kernel_size);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "right impdata_create failed: " + std::to_string(ret));
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);

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
    for (int n = 0; n < nsamples; n++) {
      conv->inpdata(0)[n] = data[2 * n];
      conv->inpdata(1)[n] = data[2 * n + 1];
    }

    int ret = conv->process(THREAD_SYNC_MODE);

    if (ret != 0) {
      util::debug(log_tag + "IR: process failed: " + std::to_string(ret));
    }

    // interleave
    for (int n = 0; n < nsamples; n++) {
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
