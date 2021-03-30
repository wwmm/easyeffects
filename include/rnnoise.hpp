/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#ifndef RNNOISE_HPP
#define RNNOISE_HPP

#include <rnnoise.h>
#include <memory>
#include <mutex>
#include "plugin_base.hpp"
#include "resampler.hpp"

class RNNoise : public PluginBase {
 public:
  RNNoise(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
  RNNoise(const RNNoise&) = delete;
  auto operator=(const RNNoise&) -> RNNoise& = delete;
  RNNoise(const RNNoise&&) = delete;
  auto operator=(const RNNoise&&) -> RNNoise& = delete;
  ~RNNoise() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  auto get_latency() const -> float;

 private:
  uint blocksize = 480;
  uint rnnoise_rate = 48000;

  bool resample = false, notify_latency = false;

  float latency = 0.0F;
  uint latency_n_frames = 0;

  std::deque<float> buffer_L, buffer_R;
  std::deque<float> deque_in_L, deque_in_R;
  std::deque<float> deque_out_L, deque_out_R;

  std::vector<float> data_L, data_R;

  std::unique_ptr<Resampler> resampler_inL, resampler_outL;
  std::unique_ptr<Resampler> resampler_inR, resampler_outR;

  RNNModel* model = nullptr;

  DenoiseState *state_left = nullptr, *state_right = nullptr;

  std::mutex rnnoise_mutex;

  auto get_model_from_file() -> RNNModel*;

  void free_rnnoise();
};

#endif
