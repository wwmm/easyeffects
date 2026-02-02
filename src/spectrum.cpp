/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "spectrum.hpp"
#include <fftw3.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <QApplication>
#include <QString>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <format>
#include <memory>
#include <mutex>
#include <numbers>
#include <span>
#include <string>
#include <tuple>
#include "easyeffects_db_spectrum.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Spectrum::Spectrum(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag, "spectrum", tags::plugin_package::Package::ee, instance_id, pipe_manager, pipe_type),
      settings(DbSpectrum::self()) {
  bypass = !DbSpectrum::state();
  // Precompute the Hann window, which is an expensive operation.
  // https://en.wikipedia.org/wiki/Hann_function
  for (size_t n = 0; n < n_bands; n++) {
    hann_window[n] =
        0.5F *
        (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) / static_cast<float>(n_bands - 1)));
  }

  complex_output = fftwf_alloc_complex(n_bands);

  plan = fftwf_plan_dft_r2c_1d(static_cast<int>(n_bands), real_input.data(), complex_output, FFTW_ESTIMATE);

  if (plan != nullptr && complex_output != nullptr) {
    fftw_ready = true;
  }

  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/comp_delay_x2_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(
        std::format("{}{} is not installed, spectrum will not have A/V sync compensation", log_tag, lv2_plugin_uri));
  }

  lv2_wrapper->set_control_port_value("mode_l", 2);
  lv2_wrapper->set_control_port_value("mode_r", 2);

  lv2_wrapper->set_control_port_value("dry_l", 0.0F);
  lv2_wrapper->set_control_port_value("dry_r", 0.0F);

  lv2_wrapper->set_control_port_value("wet_l", util::db_to_linear(0.0F));
  lv2_wrapper->set_control_port_value("wet_r", util::db_to_linear(0.0F));

  BIND_LV2_PORT("time_l", avsyncDelay, setAvsyncDelay, DbSpectrum::avsyncDelayChanged);
  BIND_LV2_PORT("time_r", avsyncDelay, setAvsyncDelay, DbSpectrum::avsyncDelayChanged);

  connect(DbSpectrum::self(), &DbSpectrum::stateChanged, [&]() { bypass = !DbSpectrum::state(); });
}

Spectrum::~Spectrum() {
  stop_worker();

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  fftw_ready = false;

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  fftwf_destroy_plan(plan);

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Spectrum::reset() {}

void Spectrum::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void Spectrum::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  std::ranges::fill(real_input, 0.0F);
  std::ranges::fill(latest_samples_mono, 0.0F);

  left_delayed_vector.resize(n_samples, 0.0F);
  right_delayed_vector.resize(n_samples, 0.0F);

  left_delayed = std::span<float>(left_delayed_vector);
  right_delayed = std::span<float>(right_delayed_vector);

  lv2_wrapper->set_n_samples(n_samples);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  ready = false;

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        util::debug(std::format("{} creating instance of comp delay x2 stereo for spectrum A/V sync", log_tag));

        lv2_wrapper->create_instance(rate);

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Spectrum::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::ranges::copy(left_in, left_out.begin());
  std::ranges::copy(right_in, right_out.begin());

  if (bypass || !fftw_ready || !ready) {
    return;
  }

  /**
   * delay the visualization of the spectrum by the reported latency of the
   * output device, so that the spectrum is visually in sync with the audio
   * as experienced by the user. (A/V sync)
   */
  if (lv2_wrapper->found_plugin && lv2_wrapper->has_instance()) {
    lv2_wrapper->connect_data_ports(left_in, right_in, left_delayed, right_delayed);
    lv2_wrapper->run();

    // Downmix the latest n_bands samples from the delayed signal.
    if (n_samples < n_bands) {
      // Drop the oldest quantum.
      std::memmove(latest_samples_mono.data(), &latest_samples_mono[n_samples], (n_bands - n_samples) * sizeof(float));

      // Copy the new quantum.
      for (size_t n = 0; n < n_samples; n++) {
        latest_samples_mono[n_bands - n_samples + n] = 0.5F * (left_delayed[n] + right_delayed[n]);
      }
    } else {
      // Copy the latest n_bands samples.
      for (size_t n = 0; n < n_bands; n++) {
        latest_samples_mono[n] =
            0.5F * (left_delayed[n_samples - n_bands + n] + right_delayed[n_samples - n_bands + n]);
      }
    }
  } else {
    // Downmix the latest n_bands samples from the non-delayed signal.
    if (n_samples < n_bands) {
      // Drop the oldest quantum.
      std::memmove(latest_samples_mono.data(), &latest_samples_mono[n_samples], (n_bands - n_samples) * sizeof(float));

      // Copy the new quantum.
      for (size_t n = 0; n < n_samples; n++) {
        latest_samples_mono[n_bands - n_samples + n] = 0.5F * (left_in[n] + right_in[n]);
      }
    } else {
      // Copy the latest n_bands samples.
      for (size_t n = 0; n < n_bands; n++) {
        latest_samples_mono[n] = 0.5F * (left_in[n_samples - n_bands + n] + right_in[n_samples - n_bands + n]);
      }
    }
  }

  /**
   * OK, we have the latest_samples_mono array that contains n_bands samples. We
   * want to export it to the GUI thread. We don't wakeup the GUI thread from
   * realtime, we only want to update the buffer and let the GUI thread follow
   * its scheduling and have access to our new buffer. We accept losing old
   * data.
   *
   * For that, we want to synchronise both threads. Realtime shouldn't have to
   * wait (ie loop or syscall). It is fine if GUI waits a little.
   *
   * The overall concept is to use two buffers. When realtime comes in, it
   * writes into one. When GUI arrives, it waits for realtime to be done
   * writing (if it is busy) and it switches the active buffer to become the
   * other one. As GUI is the one doing the toggle, it knows it can read at its
   * pace the inactive buffer. Realtime will always have a buffer to write
   * into, and it is fine if it overwrites data.
   *
   * We use an atomic integer (db_control) that contains three bits:
   *  - DB_BIT_IDX: the current buffer index.
   *  - DB_BIT_NEWDATA: indicates if realtime wrote new data.
   *  - DB_BIT_BUSY: indicates if realtime is currently busy writing data.
   *
   * Realtime does this:
   *  - Grab db_control and enable its BUSY bit.
   *  - Write data into the correct buffer based on the IDX bit.
   *  - Write db_control with same index as before, BUSY bit disabled and
   *    NEWDATA bit enabled.
   *
   * GUI does this:
   *  - Early return if NEWDATA is not enabled. It means realtime hasn't ran
   *    since the last time GUI ran.
   *  - Then it tries toggling the IDX bit. "Tries" because the operation fails
   *    as long as the BUSY bit is active.
   *  - From now on it can read the previous buffer knowing realtime cannot be
   *    writing into it.
   *
   * This is single-producer single-consumer double-buffering. The
   * implementation is inspired from the "Real-time 101" ADC19 talk
   * (David Rowland & Fabian Renn-Giles).
   *
   * Talk part 1 & 2, slides (page 74 onwards) and illustrating library:
   * https://www.youtube.com/watch?v=Q0vrQFyAdWI
   * https://www.youtube.com/watch?v=PoZAo2Vikbo
   * https://github.com/drowaudio/presentations/blob/master/ADC%202019%20-%20Real-time%20101/Real-time%20101.pdf
   * https://github.com/hogliux/farbot
   */

  // Grab the current index AND mark as busy at the same time.
  int index = db_control.fetch_or(static_cast<int>(DB_BIT::BUSY)) & static_cast<int>(DB_BIT::IDX);

  // Fill the buffer.
  db_buffers[index] = latest_samples_mono;

  // Mark new data available AND mark as not busy anymore.
  db_control.store(index | static_cast<int>(DB_BIT::NEWDATA));
}

auto Spectrum::compute_magnitudes() -> std::tuple<uint, QList<double>> {
  std::scoped_lock<std::mutex> lock(data_mutex);

  // Early return if no new data is available, ie if process() has not been
  // called since our last compute_magnitudes() call.
  int curr_control = db_control.load();
  if (!fftw_ready || !(curr_control & static_cast<int>(DB_BIT::NEWDATA))) {
    return {0, {}};
  }

  // CAS loop to toggle the buffer used and remove NEWDATA flag, waiting for !BUSY.
  int next_control = 0;
  do {
    curr_control &= ~static_cast<int>(DB_BIT::BUSY);
    next_control = (curr_control ^ static_cast<int>(DB_BIT::IDX)) & static_cast<int>(DB_BIT::IDX);
  } while (!db_control.compare_exchange_weak(curr_control, next_control));

  // Buffer with data is at the index which was found inside db_control.
  int index = curr_control & static_cast<int>(DB_BIT::IDX);
  float* buf = db_buffers[index].data();

  // https://en.wikipedia.org/wiki/Hann_function
  for (size_t n = 0; n < n_bands; n++) {
    real_input[n] = buf[n] * hann_window[n];
  }

  fftwf_execute(plan);

  for (uint i = 0U; i < output.size(); i++) {
    float sqr = (complex_output[i][0] * complex_output[i][0]) + (complex_output[i][1] * complex_output[i][1]);

    sqr /= static_cast<float>(output.size() * output.size());

    output[i] = static_cast<double>(sqr);
  }

  return {rate, output};
}

void Spectrum::process([[maybe_unused]] std::span<float>& left_in,
                       [[maybe_unused]] std::span<float>& right_in,
                       [[maybe_unused]] std::span<float>& left_out,
                       [[maybe_unused]] std::span<float>& right_out,
                       [[maybe_unused]] std::span<float>& probe_left,
                       [[maybe_unused]] std::span<float>& probe_right) {}

auto Spectrum::get_latency_seconds() -> float {
  return 0.0F;
}
