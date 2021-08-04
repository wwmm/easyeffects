/*
 *  Copyright © 2017-2021 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "spectrum.hpp"

Spectrum::Spectrum(const std::string& tag,
                   const std::string& schema,
                   const std::string& schema_path,
                   PipeManager* pipe_manager)
    : PluginBase(tag, "spectrum", schema, schema_path, pipe_manager) {
  real_input.resize(n_bands);
  output.resize(n_bands / 2U + 1U);

  complex_output = fftwf_alloc_complex(n_bands);

  plan = fftwf_plan_dft_r2c_1d(static_cast<int>(n_bands), real_input.data(), complex_output, FFTW_ESTIMATE);

  fftw_ready = true;
}

Spectrum::~Spectrum() {
  pw_thread_loop_lock(pm->thread_loop);

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);

  std::scoped_lock<std::mutex> lock(data_mutex);

  fftw_ready = false;

  fftwf_destroy_plan(plan);

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  util::debug(log_tag + name + " destroyed");
}

void Spectrum::setup() {}

void Spectrum::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (!post_messages || !fftw_ready) {
    return;
  }

  uint count = 0U;

  for (uint n = 0U, m = left_in.size(); n < m; n++) {
    uint k = total_count + n;

    if (k < real_input.size()) {
      // https://en.wikipedia.org/wiki/Hann_function

      auto w = 0.5F * (1.0F - cosf(2.0F * std::numbers::pi_v<float> * k / static_cast<float>(real_input.size() - 1)));

      real_input[k] = 0.5F * (left_in[n] + right_in[n]) * w;

      count++;
    } else {
      break;
    }
  }

  total_count += count;

  if (total_count == real_input.size()) {
    total_count = 0U;

    fftwf_execute(plan);

    for (uint i = 0U, m = output.size(); i < m; i++) {
      float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

      sqr /= static_cast<float>(n_samples * n_samples);

      output[i] = sqr;
    }

    auto output_copy = output;

    Glib::signal_idle().connect_once([=, this] { power.emit(rate, output_copy.size(), output_copy); });
  }
}
