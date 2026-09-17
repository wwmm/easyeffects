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

#include <atomic>
#include "output_level.hpp"

// A separate branch of the input pipeline keeps monitoring volume and shared
// desktop audio out of the microphone signal sent to recording applications.
class MicrophoneMonitor : public OutputLevel {
 public:
  explicit MicrophoneMonitor(pw::Manager* pm) : OutputLevel("mic_monitor: ", pm, PipelineType::input, "0") {}

  void set_volume(float value) { volume.store(value, std::memory_order_relaxed); }

  using OutputLevel::process;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override {
    const auto gain = volume.load(std::memory_order_relaxed);
    for (std::size_t i = 0; i < left_out.size(); ++i) {
      left_out[i] = left_in[i] * gain;
      right_out[i] = right_in[i] * gain;
    }
  }

 private:
  std::atomic<float> volume{1.0F};
};
