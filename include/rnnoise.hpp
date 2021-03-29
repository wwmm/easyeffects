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
#include <mutex>
#include "plugin_base.hpp"

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

 private:
  uint blocksize = 480;

  std::deque<float> rnnoise_buffer;

  std::vector<float> data_L;
  std::vector<float> data_R;

  RNNModel* model = nullptr;
  DenoiseState *state_left = nullptr, *state_right = nullptr;
};

#endif
