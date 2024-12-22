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

#pragma once

#include <fftw3.h>
#include <qlist.h>
#include <sys/types.h>
#include <QString>
#include <array>
#include <atomic>
#include <span>
#include <string>
#include <tuple>
#include <vector>
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Spectrum : public PluginBase {
 public:
  Spectrum(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Spectrum(const Spectrum&) = delete;
  auto operator=(const Spectrum&) -> Spectrum& = delete;
  Spectrum(const Spectrum&&) = delete;
  auto operator=(const Spectrum&&) -> Spectrum& = delete;
  ~Spectrum() override;

  void reset() override;

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

  auto compute_magnitudes() -> std::tuple<uint, QList<double>>;  // rate, magnitudes

 private:
  std::atomic<bool> fftw_ready = false;

  fftwf_plan plan = nullptr;

  fftwf_complex* complex_output = nullptr;

  static constexpr uint n_bands = 8192U;

  std::array<float, n_bands> real_input;

  QList<double> output = QList<double>((n_bands / 2U + 1U));

  std::vector<float> left_delayed_vector;
  std::vector<float> right_delayed_vector;
  std::span<float> left_delayed;
  std::span<float> right_delayed;

  std::array<float, n_bands> latest_samples_mono;

  std::array<float, n_bands> hann_window;

  enum {
    DB_BIT_IDX = (1 << 0),      // To which db_buffers array process() should write.
    DB_BIT_NEWDATA = (1 << 1),  // If new data has been written by process().
    DB_BIT_BUSY = (1 << 2),     // If process() is currently writing data.
  };

  std::array<std::array<float, n_bands>, 2> db_buffers;
  std::atomic<int> db_control = {0};
  static_assert(std::atomic<int>::is_always_lock_free);
};