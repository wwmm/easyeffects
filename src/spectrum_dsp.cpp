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

#include "spectrum_dsp.hpp"
#include <fftw3.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>
#include <utility>
#include <vector>

namespace spectrum_dsp {

auto make_window(WindowType type, size_t size) -> std::vector<float> {
  // a0 - a1 cos(x) + a2 cos(2x) - ...   https://en.wikipedia.org/wiki/Window_function
  std::vector<double> coefficients;

  switch (type) {
    case WindowType::rectangular:
      coefficients = {1.0};
      break;
    case WindowType::hann:
      coefficients = {0.5, 0.5};
      break;
    case WindowType::hamming:
      coefficients = {0.54, 0.46};
      break;
    case WindowType::blackman:
      coefficients = {0.42, 0.5, 0.08};
      break;
    case WindowType::blackman_harris:
      coefficients = {0.35875, 0.48829, 0.14128, 0.01168};
      break;
    case WindowType::flat_top:
      coefficients = {0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368};
      break;
  }

  std::vector<float> output(size);

  for (size_t n = 0U; n < size; n++) {
    const double x = 2.0 * std::numbers::pi * static_cast<double>(n) / static_cast<double>(size);

    double w = 0.0;

    for (size_t k = 0U; k < coefficients.size(); k++) {
      const double sign = (k % 2U == 0U) ? 1.0 : -1.0;

      w += sign * coefficients[k] * std::cos(static_cast<double>(k) * x);
    }

    output[n] = static_cast<float>(w);
  }

  return output;
}

auto band_edges(std::span<const double> centers, bool logarithmic) -> std::vector<double> {
  std::vector<double> edges;

  if (centers.empty()) {
    return edges;
  }

  edges.resize(centers.size() + 1U);

  if (centers.size() == 1U) {
    edges[0] = logarithmic ? centers[0] / std::numbers::sqrt2 : 0.5 * centers[0];
    edges[1] = logarithmic ? centers[0] * std::numbers::sqrt2 : 1.5 * centers[0];

    return edges;
  }

  for (size_t n = 1U; n < centers.size(); n++) {
    edges[n] = logarithmic ? std::sqrt(centers[n - 1U] * centers[n]) : 0.5 * (centers[n - 1U] + centers[n]);
  }

  const size_t last = centers.size() - 1U;

  if (logarithmic) {
    edges[0] = centers[0] * centers[0] / edges[1];
    edges[last + 1U] = centers[last] * centers[last] / edges[last];
  } else {
    edges[0] = std::max(0.0, (2.0 * centers[0]) - edges[1]);
    edges[last + 1U] = (2.0 * centers[last]) - edges[last];
  }

  return edges;
}

Analyzer::~Analyzer() {
  release();
}

auto Analyzer::configure(size_t fft_size, size_t zero_padding, WindowType window_function, double rate) -> bool {
  zero_padding = std::max<size_t>(zero_padding, 1U);

  if (plan != nullptr && fft_size == n_fft && zero_padding == padding && window_function == window_type &&
      rate == sample_rate) {
    return true;
  }

  release();

  if (fft_size < 2U || rate <= 0.0) {
    return false;
  }

  n_fft = fft_size;
  padding = zero_padding;
  window_type = window_function;
  sample_rate = rate;

  const size_t transform_size = n_fft * padding;
  const size_t bins = (transform_size / 2U) + 1U;

  real_input = fftwf_alloc_real(transform_size);
  complex_output = fftwf_alloc_complex(bins);

  if (real_input == nullptr || complex_output == nullptr) {
    release();

    return false;
  }

  std::fill_n(real_input, transform_size, 0.0F);

  plan = fftwf_plan_dft_r2c_1d(static_cast<int>(transform_size), real_input, complex_output, FFTW_ESTIMATE);

  if (plan == nullptr) {
    release();

    return false;
  }

  window = make_window(window_type, n_fft);

  double sum = 0.0;
  double sum_of_squares = 0.0;

  for (const auto& w : window) {
    sum += w;
    sum_of_squares += static_cast<double>(w) * w;
  }

  bin_width = sample_rate / static_cast<double>(transform_size);

  resolution_bandwidth = sample_rate * sum_of_squares / (sum * sum);

  // Parseval with window energy correction: the bin powers add up to the mean square of the signal.
  power_scale = 1.0 / (static_cast<double>(transform_size) * sum_of_squares);

  raw_power.assign(bins, 0.0);
  smoothed_power.assign(bins, 0.0);
  cumulative_power.assign(bins, 0.0);

  has_raw = false;
  has_smoothed = false;

  return true;
}

void Analyzer::release() {
  if (plan != nullptr) {
    fftwf_destroy_plan(plan);
    plan = nullptr;
  }

  if (real_input != nullptr) {
    fftwf_free(real_input);
    real_input = nullptr;
  }

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
    complex_output = nullptr;
  }

  n_fft = 0U;
  padding = 0U;
  sample_rate = 0.0;

  has_raw = false;
  has_smoothed = false;
}

void Analyzer::analyze(std::span<const float> samples) {
  if (plan == nullptr) {
    return;
  }

  const auto source = samples.size() >= n_fft ? samples.last(n_fft) : samples;
  const size_t offset = n_fft - source.size();

  std::fill_n(real_input, offset, 0.0F);

  for (size_t n = 0U; n < source.size(); n++) {
    real_input[offset + n] = source[n] * window[offset + n];
  }

  std::fill_n(&real_input[n_fft], n_fft * (padding - 1U), 0.0F);

  fftwf_execute(plan);

  const size_t last = raw_power.size() - 1U;

  for (size_t k = 0U; k <= last; k++) {
    const double re = complex_output[k][0];
    const double im = complex_output[k][1];

    double p = ((re * re) + (im * im)) * power_scale;

    // One-sided spectrum
    if (k != 0U && k != last) {
      p *= 2.0;
    }

    raw_power[k] = p;
  }

  has_raw = true;
}

void Analyzer::smooth(double dt, double attack_time, double decay_time) {
  if (!has_raw) {
    return;
  }

  if (!has_smoothed) {
    smoothed_power = raw_power;

    has_smoothed = true;
  } else {
    dt = std::max(dt, 0.0);

    // On linear power, so in dB the decay is a constant 4.34 dB per decay time
    const double k_attack = attack_time > 0.0 ? std::exp(-dt / attack_time) : 0.0;
    const double k_decay = decay_time > 0.0 ? std::exp(-dt / decay_time) : 0.0;

    for (size_t n = 0U; n < smoothed_power.size(); n++) {
      const double target = raw_power[n];
      const double current = smoothed_power[n];
      const double k = target > current ? k_attack : k_decay;

      smoothed_power[n] = target + (k * (current - target));
    }
  }

  cumulative_power[0] = 0.0;

  for (size_t n = 1U; n < smoothed_power.size(); n++) {
    cumulative_power[n] = cumulative_power[n - 1U] + (0.5 * (smoothed_power[n - 1U] + smoothed_power[n]));
  }
}

auto Analyzer::integral_to(double frequency) const -> double {
  const size_t last = smoothed_power.size() - 1U;

  const double x = std::clamp(frequency / bin_width, 0.0, static_cast<double>(last));
  const size_t k = std::min(static_cast<size_t>(x), last - 1U);
  const double t = x - static_cast<double>(k);

  const double p0 = smoothed_power[k];
  const double p1 = smoothed_power[k + 1U];

  return cumulative_power[k] + (p0 * t) + (0.5 * (p1 - p0) * t * t);
}

void Analyzer::band_levels(std::span<const double> edges, std::span<double> levels_db) const {
  const size_t n_bands = std::min(levels_db.size(), edges.empty() ? 0U : edges.size() - 1U);

  if (!has_smoothed) {
    std::fill(levels_db.begin(), levels_db.end(), minimum_db);

    return;
  }

  const double nyquist = 0.5 * sample_rate;

  for (size_t n = 0U; n < n_bands; n++) {
    double low = edges[n];
    double high = edges[n + 1U];

    if (high < low) {
      std::swap(low, high);
    }

    if (low >= nyquist) {
      levels_db[n] = minimum_db;

      continue;
    }

    low = std::max(low, 0.0);
    high = std::min(high, nyquist);

    const double width = high - low;

    double power = 0.0;

    if (width > 1e-6 * resolution_bandwidth) {
      power = integral_to(high) - integral_to(low);

      if (width < resolution_bandwidth) {
        power *= resolution_bandwidth / width;
      }
    } else {
      const double x = std::clamp(low / bin_width, 0.0, static_cast<double>(smoothed_power.size() - 1U));
      const size_t k = std::min(static_cast<size_t>(x), smoothed_power.size() - 2U);
      const double t = x - static_cast<double>(k);
      const double density = ((1.0 - t) * smoothed_power[k]) + (t * smoothed_power[k + 1U]);

      power = density * resolution_bandwidth / bin_width;
    }

    // A full scale sine has a mean square of 1/2
    const double db = power > 0.0 ? 10.0 * std::log10(2.0 * power) : minimum_db;

    levels_db[n] = std::max(db, minimum_db);
  }

  std::fill(levels_db.begin() + static_cast<std::ptrdiff_t>(n_bands), levels_db.end(), minimum_db);
}

void AutoRange::update(std::span<const double> levels_db,
                       double dt,
                       double floor_db,
                       double ceiling_db,
                       double min_span_db) {
  if (levels_db.empty()) {
    return;
  }

  sorted.assign(levels_db.begin(), levels_db.end());

  const auto low_index = static_cast<std::ptrdiff_t>(low_percentile * static_cast<double>(sorted.size() - 1U));

  std::nth_element(sorted.begin(), sorted.begin() + low_index, sorted.end());

  const double frame_low = std::clamp(sorted[low_index], floor_db, ceiling_db);
  const double frame_high = std::clamp(*std::ranges::max_element(levels_db) + top_headroom_db, floor_db, ceiling_db);

  if (!initialized) {
    smoothed_low = frame_low;
    smoothed_high = frame_high;

    initialized = true;
  } else {
    dt = std::max(dt, 0.0);

    const double k_high = std::exp(-dt / top_release_time);
    const double k_low = std::exp(-dt / bottom_time);

    smoothed_high = std::max(frame_high, frame_high + (k_high * (smoothed_high - frame_high)));
    smoothed_low = frame_low + (k_low * (smoothed_low - frame_low));
  }

  min_span_db = std::min(min_span_db, ceiling_db - floor_db);

  range_high = std::clamp(smoothed_high, floor_db + min_span_db, ceiling_db);
  range_low = std::clamp(std::min(smoothed_low, range_high - min_span_db), floor_db, range_high - min_span_db);
}

}  // namespace spectrum_dsp
