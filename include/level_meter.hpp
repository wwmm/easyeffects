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

#include <ebur128.h>
#include <sigc++/signal.h>
#include <sys/types.h>
#include <span>
#include <string>
#include <thread>
#include <vector>
#include "pipe_manager.hpp"
#include "plugin_base.hpp"

class LevelMeter : public PluginBase {
 public:
  LevelMeter(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager,
             PipelineType pipe_type);
  LevelMeter(const LevelMeter&) = delete;
  auto operator=(const LevelMeter&) -> LevelMeter& = delete;
  LevelMeter(const LevelMeter&&) = delete;
  auto operator=(const LevelMeter&&) -> LevelMeter& = delete;
  ~LevelMeter() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  auto get_latency_seconds() -> float override;

  void reset_history();

  sigc::signal<void(const double,  // momentary
                    const double,  // shortterm
                    const double,  // integrated
                    const double,  // relative
                    const double,  // range
                    const double,  // true_peak_R
                    const double   // true_peak_R
                    )>
      results;  // range

 private:
  bool ebur128_ready = false;

  uint old_rate = 0U;

  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;
  double range = 0.0;

  double true_peak_L = 0.0;
  double true_peak_R = 0.0;

  std::vector<float> data;

  ebur128_state* ebur_state = nullptr;

  std::vector<std::thread> mythreads;

  auto init_ebur128() -> bool;
};
