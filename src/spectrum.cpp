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
  deque_in_mono.resize(0U);

  std::ranges::fill(real_input, 0.0F);

  /*
    real_input size is hardcoded to 8192. THe same maxium buffer size hardcoded in PipeWire
    https://gitlab.freedesktop.org/pipewire/pipewire/-/blob/master/src/pipewire/filter.c#L48. If we reevei a smaller
    array we have to insert some zeros in the beginning.
  */

  if (n_samples < real_input.size()) {
    while (deque_in_mono.size() != real_input.size() - n_samples) {
      deque_in_mono.push_back(0.0F);
    }
  }
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

  for (uint n = 0U; n < left_in.size(); n++) {
    deque_in_mono.push_back(0.5F * (left_in[n] + right_in[n]));
  }

  for (size_t n = 0; n < deque_in_mono.size(); n++) {
    if (n < real_input.size()) {
      // https :  // en.wikipedia.org/wiki/Hann_function

      const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                              static_cast<float>(real_input.size() - 1U)));

      real_input[n] = deque_in_mono[n] * w;
    }
  }

  size_t count = 0U;

  while (count < n_samples && !deque_in_mono.empty()) {
    deque_in_mono.pop_front();

    count++;
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
