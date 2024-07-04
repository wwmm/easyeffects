/*
 *  Copyright © 2017-2024 Wellington Wallace
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
  real_input.resize(n_bands);
  output.resize(n_bands / 2U + 1U);

  latest_samples_mono.resize(n_bands);

  // Precompute the Hann window, which is an expensive operation.
  // https://en.wikipedia.org/wiki/Hann_function
  hann_window.resize(n_bands);
  for (size_t n = 0; n < n_bands; n++) {
    hann_window[n] = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> *
        static_cast<float>(n) / static_cast<float>(n_bands-1)));
  }

  complex_output = fftwf_alloc_complex(n_bands);

  plan = fftwf_plan_dft_r2c_1d(static_cast<int>(n_bands), real_input.data(), complex_output, FFTW_ESTIMATE);

  g_signal_connect(settings, "changed::show", G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                     auto* self = static_cast<Spectrum*>(user_data);

                     std::scoped_lock<std::mutex> lock(self->data_mutex);

                     self->bypass = g_settings_get_boolean(settings, key) == 0;
                   }),
                   this);
}

Spectrum::~Spectrum() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

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
}

void Spectrum::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (bypass || !fftw_ready) {
    return;
  }

  if (n_samples < n_bands) {
    // Drop the oldest quantum.
    for (size_t n = 0; n < n_bands - n_samples; n++)
      latest_samples_mono[n] = latest_samples_mono[n_samples + n];

    // Copy the new quantum.
    for (size_t n = 0; n < n_samples; n++)
      latest_samples_mono[n_bands - n_samples + n] = 0.5F * (left_in[n] + right_in[n]);
  } else {
    // Copy the latest n_bands samples.
    for (size_t n = 0; n < n_bands; n++)
      latest_samples_mono[n] = 0.5F * (left_in[n_samples - n_bands + n] +
                                       right_in[n_samples - n_bands + n]);
  }

  for (size_t n = 0; n < n_bands; n++) {
    real_input[n] = latest_samples_mono[n] * hann_window[n];
  }

  if (send_notifications) {
    util::idle_add([this]() {
      if (bypass) {
        return;
      }

      fftwf_execute(plan);

      for (uint i = 0U; i < output.size(); i++) {
        float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

        sqr /= static_cast<float>(output.size() * output.size());

        output[i] = static_cast<double>(sqr);
      }

      power.emit(rate, output.size(), output);
    });
  }
}

auto Spectrum::get_latency_seconds() -> float {
  return 0.0F;
}
