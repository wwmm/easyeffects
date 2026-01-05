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

#include <api/audio/audio_processing.h>
#include <api/scoped_refptr.h>
#include <qobject.h>
#include <qtypes.h>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_echo_canceller.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class EchoCanceller : public PluginBase {
 public:
  EchoCanceller(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  EchoCanceller(const EchoCanceller&) = delete;
  auto operator=(const EchoCanceller&) -> EchoCanceller& = delete;
  EchoCanceller(const EchoCanceller&&) = delete;
  auto operator=(const EchoCanceller&&) -> EchoCanceller& = delete;
  ~EchoCanceller() override;

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

 private:
  db::EchoCanceller* settings = nullptr;

  bool notify_latency = false;
  bool ready = false;

  uint latency_n_frames = 0U;
  uint blocksize;

  std::vector<float> near_L, near_R;
  std::vector<float> far_L, far_R;

  std::vector<float> buf_near_L, buf_near_R;
  std::vector<float> buf_far_L, buf_far_R;
  std::vector<float> buf_out_L, buf_out_R;

  webrtc::AudioProcessing::Config ap_cfg;

  rtc::scoped_refptr<webrtc::AudioProcessing> ap_builder;

  webrtc::StreamConfig stream_config;

  void init_webrtc();
};
