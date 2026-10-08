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

#pragma once

#include <fftw3.h>
#include <cstddef>
#include <span>
#include <vector>

namespace spectrum_dsp {

// The order matches the windowFunction choices in easyeffects_db_spectrum.kcfg
enum class WindowType { rectangular, hann, hamming, blackman, blackman_harris, flat_top };

// Periodic variant, as needed for spectral analysis
auto make_window(WindowType type, size_t size) -> std::vector<float>;

// Returns centers.size() + 1 edges, placed at the geometric (log) or arithmetic (linear) means between the centers.
auto band_edges(std::span<const double> centers, bool logarithmic) -> std::vector<double>;

/**
 * End of the analysis window inside a capture buffer whose last block_size samples arrived elapsed seconds ago.
 * PipeWire delivers a whole block at once, so sliding the window through it with time gives a new spectrum on every
 * displayed frame instead of only once per quantum.
 */
auto window_end(size_t buffer_size, size_t block_size, double elapsed, double rate) -> size_t;

/**
 * Band levels are in dBFS relative to a full scale sine, independent of FFT size, zero padding and window.
 * Not thread safe. The caller must serialize configure() and release() because FFTW planning is not thread safe.
 */
class Analyzer {
 public:
  Analyzer() = default;
  Analyzer(const Analyzer&) = delete;
  auto operator=(const Analyzer&) -> Analyzer& = delete;
  Analyzer(const Analyzer&&) = delete;
  auto operator=(const Analyzer&&) -> Analyzer& = delete;
  ~Analyzer();

  auto configure(size_t fft_size, size_t zero_padding, WindowType window, double rate) -> bool;

  void release();

  void analyze(std::span<const float> samples);

  // Times in seconds. Call it every displayed frame, also without new samples, so the decay keeps moving.
  void smooth(double dt, double attack_time, double decay_time);

  /**
   * Integrates the linearly interpolated power density over each band. Bands narrower than the resolution bandwidth
   * are scaled up to it, otherwise tones would be shown too low at the bottom of a logarithmic axis.
   */
  void band_levels(std::span<const double> edges, std::span<double> levels_db) const;

  [[nodiscard]] auto has_data() const -> bool { return has_smoothed; }

  static constexpr double minimum_db = -200.0;

 private:
  size_t n_fft = 0U;
  size_t padding = 0U;
  WindowType window_type = WindowType::hann;
  double sample_rate = 0.0;

  double bin_width = 0.0;
  double resolution_bandwidth = 0.0;  // equivalent noise bandwidth in Hz
  double power_scale = 0.0;

  bool has_raw = false;
  bool has_smoothed = false;

  fftwf_plan plan = nullptr;
  float* real_input = nullptr;
  fftwf_complex* complex_output = nullptr;

  std::vector<float> window;
  std::vector<double> raw_power;
  std::vector<double> smoothed_power;
  std::vector<double> cumulative_power;

  [[nodiscard]] auto integral_to(double frequency) const -> double;
};

/**
 * Display range that follows the loudest band and a low percentile of the bands. The top jumps up instantly so peaks
 * are never cut off and falls slowly, the bottom moves slowly, so the range does not pump with every frame.
 */
class AutoRange {
 public:
  void update(std::span<const double> levels_db, double dt, double floor_db, double ceiling_db, double min_span_db);

  void reset() { initialized = false; }

  [[nodiscard]] auto low() const -> double { return range_low; }

  [[nodiscard]] auto high() const -> double { return range_high; }

  // Percentile instead of the minimum, so bands above a codec lowpass do not drag the bottom down
  static constexpr double low_percentile = 0.1;

  static constexpr double top_headroom_db = 3.0;
  static constexpr double top_release_time = 2.0;
  static constexpr double bottom_time = 1.5;

 private:
  bool initialized = false;

  double smoothed_low = 0.0;
  double smoothed_high = 0.0;

  double range_low = 0.0;
  double range_high = 0.0;

  std::vector<double> sorted;
};

}  // namespace spectrum_dsp
