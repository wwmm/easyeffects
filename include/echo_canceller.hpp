/*
 *  Copyright © 2017-2023 Wellington Wallace
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
#include <deque>
#include "plugin_base.hpp"

#ifdef SPEEX_AVAILABLE
#include <speex/speex_preprocess.h>
#endif

class EchoCanceller : public PluginBase {
 public:
  EchoCanceller(const std::string& tag,
                const std::string& schema,
                const std::string& schema_path,
                PipeManager* pipe_manager);
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

  uint blocksize = 512U;
  uint blocksize_ms = 20U;
  uint filter_length_ms = 100U;
  uint latency_n_frames = 0U;
  int residual_echo_suppression = -10;

  float latency_value = 0.0F;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1);

  std::vector<spx_int16_t> data_L;
  std::vector<spx_int16_t> data_R;
  std::vector<spx_int16_t> probe_L;
  std::vector<spx_int16_t> probe_R;
  std::vector<spx_int16_t> filtered_L;
  std::vector<spx_int16_t> filtered_R;

  std::deque<float> deque_out_L, deque_out_R;

  SpeexEchoState* echo_state_L = nullptr;
  SpeexEchoState* echo_state_R = nullptr;

#ifdef SPEEX_AVAILABLE

  SpeexPreprocessState *state_left = nullptr, *state_right = nullptr;

  void free_speex();

#endif

  void init_speex();
};
