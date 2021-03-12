/*
 *  Copyright © 2017-2021 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "spectrum.hpp"

Spectrum::Spectrum(const std::string& tag,
                   const std::string& schema,
                   const std::string& schema_path,
                   PipeManager* pipe_manager)
    : PluginBase(tag, "spectrum", schema, schema_path, pipe_manager) {
  fft_left_in.resize(n_bands);
  fft_right_in.resize(n_bands);
  output.resize(n_bands / 2 + 1);

  complex_left = fftwf_alloc_complex(n_bands);
  complex_right = fftwf_alloc_complex(n_bands);

  plan_l = fftwf_plan_dft_r2c_1d(n_bands, fft_left_in.data(), complex_left, FFTW_ESTIMATE);
  plan_r = fftwf_plan_dft_r2c_1d(n_bands, fft_right_in.data(), complex_right, FFTW_ESTIMATE);

  bypass = !settings->get_boolean("show");

  settings->signal_changed("show").connect([=, this](auto key) { bypass = !settings->get_boolean(key); });
}

Spectrum::~Spectrum() {
  fftwf_destroy_plan(plan_l);
  fftwf_destroy_plan(plan_r);

  if (complex_left != nullptr) {
    fftwf_free(complex_left);
  }

  if (complex_right == nullptr) {
    fftwf_free(complex_right);
  }
}

void Spectrum::setup() {}

void Spectrum::process(const std::vector<float>& left_in,
                       const std::vector<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (bypass) {
    return;
  }

  uint count = 0;

  for (uint n = 0; n < n_samples; n++) {
    uint k = total_count + n;

    if (k < n_bands) {
      fft_left_in[k] = left_in[n];
      fft_right_in[k] = right_in[n];

      count++;
    } else {
      break;
    }
  }

  total_count += count;

  if (total_count == n_bands) {
    total_count = 0;

    fftwf_execute(plan_l);
    fftwf_execute(plan_r);

    for (uint i = 0; i < output.size(); i++) {
      float sqr_l = complex_left[i][0] * complex_left[i][0] + complex_left[i][1] * complex_left[i][1];
      float sqr_r = complex_right[i][0] * complex_right[i][0] + complex_right[i][1] * complex_right[i][1];

      sqr_l /= static_cast<float>(n_samples * n_samples);
      sqr_r /= static_cast<float>(n_samples * n_samples);

      float v = 10.0F * log10f(0.5F * (sqr_l + sqr_r));

      if (!std::isinf(v)) {
        output[i] = (v > util::minimum_db_level) ? v : util::minimum_db_level;
      } else {
        output[i] = util::minimum_db_level;
      }

      // std::cout << output[i] << std::endl;
    }

    Glib::signal_idle().connect_once([=, this] { power.emit(rate, n_bands, output); });
  }
}