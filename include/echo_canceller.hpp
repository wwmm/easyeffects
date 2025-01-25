/*
 *  Copyright © 2017-2025 Wellington Wallace
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

#include <speex/speex_echo.h>
#include <climits>
#include <span>
#include <string>
#include <vector>
#include "pipe_manager.hpp"
#include "plugin_base.hpp"

#include <speex/speex_preprocess.h>
#include <speex/speexdsp_config_types.h>
#include <sys/types.h>

class EchoCanceller : public PluginBase {
 public:
  EchoCanceller(const std::string& tag,
                const std::string& schema,
                const std::string& schema_path,
                PipeManager* pipe_manager,
                PipelineType pipe_type);
  EchoCanceller(const EchoCanceller&) = delete;
  auto operator=(const EchoCanceller&) -> EchoCanceller& = delete;
  EchoCanceller(const EchoCanceller&&) = delete;
  auto operator=(const EchoCanceller&&) -> EchoCanceller& = delete;
  ~EchoCanceller() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  auto get_latency_seconds() -> float override;

 private:
  bool notify_latency = false;
  bool ready = false;

  uint filter_length_ms = 100U;
  uint latency_n_frames = 0U;

  int residual_echo_suppression = -10;
  int near_end_suppression = -10;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1.0F);

  std::vector<spx_int16_t> data_L;
  std::vector<spx_int16_t> data_R;
  std::vector<spx_int16_t> probe_mono;
  std::vector<spx_int16_t> filtered_L;
  std::vector<spx_int16_t> filtered_R;

  SpeexEchoState* echo_state_L = nullptr;
  SpeexEchoState* echo_state_R = nullptr;

  SpeexPreprocessState *state_left = nullptr, *state_right = nullptr;

  void free_speex();

  void init_speex();
};
