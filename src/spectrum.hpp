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

#include <sys/types.h>
#include <QString>
#include <array>
#include <atomic>
#include <chrono>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_spectrum.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "spectrum_dsp.hpp"

class Spectrum : public PluginBase {
 public:
  Spectrum(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Spectrum(const Spectrum&) = delete;
  auto operator=(const Spectrum&) -> Spectrum& = delete;
  Spectrum(const Spectrum&&) = delete;
  auto operator=(const Spectrum&&) -> Spectrum& = delete;
  ~Spectrum() override;

  void reset() override;

  void clear_data() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  auto get_latency_seconds() -> float override;

  // Returns false when there is nothing to show yet
  auto compute_band_levels(std::span<const double> band_edges, std::span<double> levels_db) -> bool;

 private:
  DbSpectrum* settings = nullptr;

  static constexpr uint max_fft_size = 16384U;
  static constexpr uint max_quantum = 8192U;
  static constexpr uint capture_size = max_fft_size + max_quantum;

  bool ready = false;

  spectrum_dsp::Analyzer analyzer;

  std::optional<std::chrono::steady_clock::time_point> last_compute_time;

  std::vector<float> left_delayed_vector;
  std::vector<float> right_delayed_vector;
  std::span<float> left_delayed;
  std::span<float> right_delayed;

  std::array<float, capture_size> latest_samples_mono;

  enum class DB_BIT {
    IDX = (1 << 0),      // To which db_buffers array process() should write.
    NEWDATA = (1 << 1),  // If new data has been written by process().
    BUSY = (1 << 2),     // If process() is currently writing data.
  };

  struct CaptureBuffer {
    std::array<float, capture_size> samples;
    uint block_size = 0U;
    std::chrono::steady_clock::time_point block_time;
  };

  std::array<CaptureBuffer, 2> db_buffers;
  int gui_buffer_index = -1;  // The buffer compute_band_levels() owns after the last swap
  std::atomic<int> db_control = {0};
  static_assert(std::atomic<int>::is_always_lock_free);
};
