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

#include <ebur128.h>
#include <sys/types.h>
#include <QString>
#include <span>
#include <string>
#include <thread>
#include <vector>
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class AutoGain : public PluginBase {
 public:
  AutoGain(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  AutoGain(const AutoGain&) = delete;
  auto operator=(const AutoGain&) -> AutoGain& = delete;
  AutoGain(const AutoGain&&) = delete;
  auto operator=(const AutoGain&&) -> AutoGain& = delete;
  ~AutoGain() override;

  enum class Reference {
    momentary,
    shortterm,
    integrated,
    geometric_mean_msi,
    geometric_mean_ms,
    geometric_mean_mi,
    geometric_mean_si
  };

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

  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;
  double range = 0.0;
  double loudness = 0.0;

 private:
  bool ebur128_ready = false;

  uint old_rate = 0U;

  double target = -23.0;  // target loudness level
  double silence_threshold = -70.0;
  double internal_output_gain = 1.0;

  Reference reference = Reference::geometric_mean_msi;

  std::vector<float> data;

  ebur128_state* ebur_state = nullptr;

  std::vector<std::thread> mythreads;

  auto init_ebur128() -> bool;

  static auto parse_reference_key(const std::string& key) -> Reference;

  void set_maximum_history(const int& seconds);
};