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

#include <ebur128.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <sys/types.h>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_level_meter.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class LevelMeter : public PluginBase {
  Q_OBJECT

 public:
  LevelMeter(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_i);
  LevelMeter(const LevelMeter&) = delete;
  auto operator=(const LevelMeter&) -> LevelMeter& = delete;
  LevelMeter(const LevelMeter&&) = delete;
  auto operator=(const LevelMeter&&) -> LevelMeter& = delete;
  ~LevelMeter() override;

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

  Q_INVOKABLE [[nodiscard]] float getMomentaryLevel() const;

  Q_INVOKABLE [[nodiscard]] float getShorttermLevel() const;

  Q_INVOKABLE [[nodiscard]] float getIntegratedLevel() const;

  Q_INVOKABLE [[nodiscard]] float getRelativeLevel() const;

  Q_INVOKABLE [[nodiscard]] float getRangeLevel() const;

  Q_INVOKABLE [[nodiscard]] float getTruePeakL() const;

  Q_INVOKABLE [[nodiscard]] float getTruePeakR() const;

  Q_INVOKABLE void resetHistory();

 private:
  db::LevelMeter* settings = nullptr;

  bool ebur128_ready = false;

  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;
  double range = 0.0;

  double true_peak_L = 0.0;
  double true_peak_R = 0.0;

  std::vector<float> data;

  ebur128_state* ebur_state = nullptr;

  auto init_ebur128() -> bool;
};
