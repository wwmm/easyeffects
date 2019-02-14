#include "lowpass.hpp"

#define CONVPROC_SCHEDULER_PRIORITY 0
#define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
#define THREAD_SYNC_MODE true

Lowpass::Lowpass(const int& num_samples) : nsamples(num_samples) {
  conv = new Convproc();

  kernel_L = new float[nsamples];
  kernel_R = new float[nsamples];

  bool failed = false;
  float density = 0.0f;
  int ret;
  unsigned int options = 0;

  options |= Convproc::OPT_FFTW_MEASURE;
  options |= Convproc::OPT_VECTOR_MODE;

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
  } else {
    util::debug(log_tag + "initialized zita-convolver engine");
  }

  ret = conv->impdata_create(0, 0, 1, kernel_L, 0, kernel_size);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "left impdata_create failed: " + std::to_string(ret));
  } else {
    util::debug(log_tag + "left impdata_create success");
  }

  ret = conv->impdata_create(1, 1, 1, kernel_R, 0, kernel_size);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag +
                "right impdata_create failed: " + std::to_string(ret));
  } else {
    util::debug(log_tag + "right impdata_create success");
  }

  ret = conv->start_process(CONVPROC_SCHEDULER_PRIORITY,
                            CONVPROC_SCHEDULER_CLASS);

  if (ret != 0) {
    failed = true;
    util::debug(log_tag + "start_process failed: " + std::to_string(ret));
  } else {
    util::debug(log_tag + "start_process success");
  }

  if (!failed) {
    ready = true;
  } else {
    ready = false;
  }
}

Lowpass::~Lowpass() {
  if (conv != nullptr) {
    if (conv->state() != Convproc::ST_STOP) {
      conv->stop_process();

      conv->cleanup();

      delete conv;

      conv = nullptr;
    }
  }

  if (kernel_L != nullptr) {
    delete[] kernel_L;
  }

  if (kernel_R != nullptr) {
    delete[] kernel_R;
  }
}
