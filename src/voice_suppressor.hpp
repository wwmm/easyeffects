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
#include <qtmetamacros.h>
#include <qtypes.h>
#include <QString>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_voice_suppressor.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class VoiceSuppressor : public PluginBase {
  Q_OBJECT

 public:
  VoiceSuppressor(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  VoiceSuppressor(const VoiceSuppressor&) = delete;
  auto operator=(const VoiceSuppressor&) -> VoiceSuppressor& = delete;
  VoiceSuppressor(const VoiceSuppressor&&) = delete;
  auto operator=(const VoiceSuppressor&&) -> VoiceSuppressor& = delete;
  ~VoiceSuppressor() override;

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

 private:
  db::VoiceSuppressor* settings = nullptr;

  bool ready = false;
  bool notify_latency = false;

  uint fft_size = 0U;
  uint hop = 0U;
  uint latency_n_frames = 0U;

  double block_time = 0.0;

  double* realL = nullptr;
  double* realR = nullptr;

  static constexpr auto epsilon = 1e-12;

  fftw_complex* complexL = nullptr;
  fftw_complex* complexR = nullptr;

  fftw_plan_s* planL = nullptr;
  fftw_plan_s* planR = nullptr;

  fftw_plan_s* planInvL = nullptr;
  fftw_plan_s* planInvR = nullptr;

  std::vector<float> freqs;

  std::vector<double> hanning_window;

  std::vector<float> buf_in_L, buf_in_R;
  std::vector<float> buf_out_L, buf_out_R;

  std::vector<float> data_L;
  std::vector<float> data_R;

  std::vector<float> ola_L;
  std::vector<float> ola_R;

  std::vector<double> fft_mag_L, fft_mag_R;

  std::vector<double> fft_cross_real;
  std::vector<double> fft_cross_img;

  std::vector<double> previous_wrapped, previous_unwrapped;

  void free_fftw();

  auto compute_local_kurtosis(int k, double* magnitude_spectrum) const -> double;

  auto calc_instantaneous_frequency(const int& k) -> double;

  static auto sigmoid(const double& x) -> double;
};
