/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects
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

#include "convolver_kernel_fft.hpp"
#include <fftw3.h>
#include <qlist.h>
#include <qpoint.h>
#include <qtypes.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <mutex>
#include <numbers>
#include <vector>
#include "util.hpp"

ConvolverKernelFFT::ConvolverKernelFFT() = default;

ConvolverKernelFFT::~ConvolverKernelFFT() {
  clear_data();
}

auto ConvolverKernelFFT::calculate_fft(std::vector<float> kernel_L,
                                       std::vector<float> kernel_R,
                                       float kernel_rate,
                                       int interp_points) -> void {
  if (kernel_L.empty() || kernel_R.empty() || kernel_L.size() != kernel_R.size()) {
    util::debug("Aborting the impulse fft calculation...");
    return;
  }

  util::debug("Calculating the impulse fft...");

  // Calculate FFT magnitudes
  auto spectrum_L = compute_fft_magnitude(kernel_L);
  auto spectrum_R = compute_fft_magnitude(kernel_R);

  // Initialize frequency axis
  std::vector<double> freq_axis(spectrum_L.size());
  for (uint n = 0U; n < freq_axis.size(); n++) {
    freq_axis[n] = 0.5F * kernel_rate * static_cast<float>(n) / static_cast<float>(freq_axis.size());
  }

  // Remove DC component at f = 0 Hz
  freq_axis.erase(freq_axis.begin());
  spectrum_L.erase(spectrum_L.begin());
  spectrum_R.erase(spectrum_R.begin());

  // Initialize linear axis
  auto linear_freq_axis = util::linspace(freq_axis.front(), freq_axis.back(), interp_points);
  auto linear_spectrum_L = util::interpolate(freq_axis, spectrum_L, linear_freq_axis);
  auto linear_spectrum_R = util::interpolate(freq_axis, spectrum_R, linear_freq_axis);

  // Initialize logarithmic frequency axis
  const auto max_freq = std::ranges::max(freq_axis);
  const auto min_freq = std::ranges::min(freq_axis);

  util::debug(std::format("Min fft frequency: {}", min_freq));
  util::debug(std::format("Max fft frequency: {}", max_freq));

  auto log_freq_axis = util::logspace(min_freq, max_freq, interp_points);
  auto log_spectrum_L = util::interpolate(freq_axis, spectrum_L, log_freq_axis);
  auto log_spectrum_R = util::interpolate(freq_axis, spectrum_R, log_freq_axis);

  // Normalizing the spectrum
  normalize_spectrum(linear_spectrum_L);
  normalize_spectrum(linear_spectrum_R);
  normalize_spectrum(log_spectrum_L);
  normalize_spectrum(log_spectrum_R);

  // Update chart data
  linear_L.resize(interp_points);
  linear_R.resize(interp_points);
  log_L.resize(interp_points);
  log_R.resize(interp_points);

  for (qsizetype n = 0; n < interp_points; n++) {
    linear_L[n] = QPointF(linear_freq_axis[n], linear_spectrum_L[n]);
    linear_R[n] = QPointF(linear_freq_axis[n], linear_spectrum_R[n]);
    log_L[n] = QPointF(log_freq_axis[n], log_spectrum_L[n]);
    log_R[n] = QPointF(log_freq_axis[n], log_spectrum_R[n]);
  }
}

auto ConvolverKernelFFT::clear_data() -> void {
  linear_L.clear();
  linear_R.clear();
  log_L.clear();
  log_R.clear();
}

auto ConvolverKernelFFT::apply_hanning_window(std::vector<float>& signal) -> void {
  for (uint n = 0U; n < signal.size(); n++) {
    const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(n) /
                                            static_cast<float>(signal.size() - 1U)));
    signal[n] *= w;
  }
}

auto ConvolverKernelFFT::compute_fft_magnitude(std::vector<float>& kernel) -> std::vector<double> {
  std::scoped_lock<std::mutex> lock(util::fftw_lock());

  if (kernel.empty() || kernel.size() < 2) {
    return {};
  }

  apply_hanning_window(kernel);

  std::vector<double> spectrum((kernel.size() / 2U) + 1U);

  auto* complex_output = fftw_alloc_complex(spectrum.size());

  if (complex_output == nullptr) {
    util::debug("FFTW complex_output allocation failed!");

    return {};
  }

  auto* real_input = static_cast<double*>(fftw_malloc(sizeof(double) * kernel.size()));

  if (real_input == nullptr) {
    util::debug("FFTW real_input allocation failed!");

    return {};
  }

  for (size_t n = 0; n < kernel.size(); n++) {
    real_input[n] = static_cast<double>(kernel[n]);
  }

  auto* plan = fftw_plan_dft_r2c_1d(static_cast<int>(kernel.size()), real_input, complex_output, FFTW_ESTIMATE);

  if (plan == nullptr) {
    util::debug("FFTW plan creation failed!");

    fftw_free(complex_output);
    fftw_free(real_input);

    return {};
  }

  fftw_execute(plan);

  for (uint i = 0U; i < spectrum.size(); i++) {
    double sqr = (complex_output[i][0] * complex_output[i][0]) + (complex_output[i][1] * complex_output[i][1]);
    sqr /= static_cast<double>(spectrum.size() * spectrum.size());
    spectrum[i] = sqr;
  }

  fftw_destroy_plan(plan);
  fftw_free(complex_output);
  fftw_free(real_input);

  return spectrum;
}

auto ConvolverKernelFFT::normalize_spectrum(std::vector<double>& spectrum) -> void {
  if (spectrum.empty()) {
    return;
  }

  const auto min_val = std::ranges::min(spectrum);
  const auto max_val = std::ranges::max(spectrum);

  if (max_val > min_val) {
    for (auto& value : spectrum) {
      value = (value - min_val) / (max_val - min_val);
    }
  }
}
