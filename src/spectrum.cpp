/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "spectrum.hpp"
#include <fftw3.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <sys/types.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <numbers>
#include <span>
#include <string>
#include "pipe_manager.hpp"
#include "plugin_base.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Spectrum::Spectrum(const std::string& tag,
                   const std::string& schema,
                   const std::string& schema_path,
                   PipeManager* pipe_manager,
                   PipelineType pipe_type)
    : PluginBase(tag, "spectrum", tags::plugin_package::ee, schema, schema_path, pipe_manager, pipe_type),
      fftw_ready(true) {
  // Precompute the Hann window, which is an expensive operation.
  // https://en.wikipedia.org/wiki/Hann_function
  for (size_t n = 0; n < n_bands; n++) {
    hann_window[n] =
        0.5F *
        (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) / static_cast<float>(n_bands - 1)));
  }

  complex_output = fftwf_alloc_complex(n_bands);

  plan = fftwf_plan_dft_r2c_1d(static_cast<int>(n_bands), real_input.data(), complex_output, FFTW_ESTIMATE);

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/comp_delay_x2_stereo");

  package_installed = lv2_wrapper->found_plugin;

  if (!package_installed) {
    util::debug(log_tag +
                "http://lsp-plug.in/plugins/lv2/comp_delay_x2_stereo is not installed, spectrum will not have A/V sync "
                "compensation");
  }

  lv2_wrapper->set_control_port_value("mode_l", 2);
  lv2_wrapper->set_control_port_value("mode_r", 2);

  lv2_wrapper->set_control_port_value("dry_l", 0.0F);
  lv2_wrapper->set_control_port_value("dry_r", 0.0F);

  lv2_wrapper->set_control_port_value("wet_l", static_cast<float>(util::db_to_linear(0.0F)));
  lv2_wrapper->set_control_port_value("wet_r", static_cast<float>(util::db_to_linear(0.0F)));

  lv2_wrapper->bind_key_int<"time_l", "avsync-delay">(settings);
  lv2_wrapper->bind_key_int<"time_r", "avsync-delay">(settings);

  g_signal_connect(settings, "changed::show", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto* self = static_cast<Spectrum*>(user_data);

                     self->bypass = g_settings_get_boolean(settings, key) == 0;
                   }),
                   this);
}

Spectrum::~Spectrum() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  fftw_ready = false;

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  fftwf_destroy_plan(plan);

  util::debug(log_tag + name + " destroyed");
}

void Spectrum::setup() {
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

  if (lv2_wrapper->get_rate() != rate) {
    util::debug(log_tag + " creating instance of comp delay x2 stereo for spectrum A/V sync");
    lv2_wrapper->create_instance(rate);
  }
}

void Spectrum::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (bypass || !fftw_ready) {
    return;
  }

  // delay the visualization of the spectrum by the reported latency
  // of the output device, so that the spectrum is visually in sync
  // with the audio as experienced by the user. (A/V sync)
  if (lv2_wrapper->found_plugin && lv2_wrapper->has_instance()) {
    lv2_wrapper->connect_data_ports(left_in, right_in, left_delayed, right_delayed);
    lv2_wrapper->run();

    // Downmix the latest n_bands samples from the delayed signal.
    if (n_samples < n_bands) {
      // Drop the oldest quantum.
      std::memmove(&latest_samples_mono[0], &latest_samples_mono[n_samples], (n_bands - n_samples) * sizeof(float));

      // Copy the new quantum.
      for (size_t n = 0; n < n_samples; n++) {
        latest_samples_mono[n_bands - n_samples + n] = 0.5F * (left_delayed[n] + right_delayed[n]);
      }
    } else {
      // Copy the latest n_bands samples.
      for (size_t n = 0; n < n_bands; n++)
        latest_samples_mono[n] =
            0.5F * (left_delayed[n_samples - n_bands + n] + right_delayed[n_samples - n_bands + n]);
    }
  } else {
    // Downmix the latest n_bands samples from the non-delayed signal.
    if (n_samples < n_bands) {
      // Drop the oldest quantum.
      std::memmove(&latest_samples_mono[0], &latest_samples_mono[n_samples], (n_bands - n_samples) * sizeof(float));

      // Copy the new quantum.
      for (size_t n = 0; n < n_samples; n++) {
        latest_samples_mono[n_bands - n_samples + n] = 0.5F * (left_in[n] + right_in[n]);
      }
    } else {
      // Copy the latest n_bands samples.
      for (size_t n = 0; n < n_bands; n++)
        latest_samples_mono[n] = 0.5F * (left_in[n_samples - n_bands + n] + right_in[n_samples - n_bands + n]);
    }
  }

  /*
   * OK, we have the latest_samples_mono array that contains n_bands samples. We
   * want to export it to the GUI thread. We don't wakeup the GUI thread from
   * realtime, we only want to update the buffer and let the GUI thread follow
   * its scheduling and have access to our new buffer. We accept losing old data.
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
  int index = db_control.fetch_or(DB_BIT_BUSY) & DB_BIT_IDX;

  // Fill the buffer.
  db_buffers[index] = latest_samples_mono;

  // Mark new data available AND mark as not busy anymore.
  db_control.store(index | DB_BIT_NEWDATA);
}

std::tuple<uint, uint, double*> Spectrum::compute_magnitudes() {
  // Early return if no new data is available, ie if process() has not been
  // called since our last compute_magnitudes() call.
  int curr_control = db_control.load();
  if (!(curr_control & DB_BIT_NEWDATA)) {
    return std::tuple<uint, uint, double*>(0, 0, nullptr);
  }

  // CAS loop to toggle the buffer used and remove NEWDATA flag, waiting for !BUSY.
  int next_control;
  do {
    curr_control &= ~DB_BIT_BUSY;
    next_control = (curr_control ^ DB_BIT_IDX) & DB_BIT_IDX;
  } while (!db_control.compare_exchange_weak(curr_control, next_control));

  // Buffer with data is at the index which was found inside db_control.
  int index = curr_control & DB_BIT_IDX;
  float* buf = db_buffers[index].data();

  // https://en.wikipedia.org/wiki/Hann_function
  for (size_t n = 0; n < n_bands; n++) {
    real_input[n] = buf[n] * hann_window[n];
  }

  fftwf_execute(plan);

  for (uint i = 0U; i < output.size(); i++) {
    float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

    sqr /= static_cast<float>(output.size() * output.size());

    output[i] = static_cast<double>(sqr);
  }

  return std::tuple<uint, uint, double*>(rate, output.size(), output.data());
}

auto Spectrum::get_latency_seconds() -> float {
  return 0.0F;
}
