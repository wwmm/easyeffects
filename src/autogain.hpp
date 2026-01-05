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
#include <qtmetamacros.h>
#include <sys/types.h>
#include <QString>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_autogain.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Autogain : public PluginBase {
  Q_OBJECT

 public:
  Autogain(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Autogain(const Autogain&) = delete;
  auto operator=(const Autogain&) -> Autogain& = delete;
  Autogain(const Autogain&&) = delete;
  auto operator=(const Autogain&&) -> Autogain& = delete;
  ~Autogain() override;

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

  Q_INVOKABLE [[nodiscard]] float getMomentaryLevel() const;

  Q_INVOKABLE [[nodiscard]] float getShorttermLevel() const;

  Q_INVOKABLE [[nodiscard]] float getIntegratedLevel() const;

  Q_INVOKABLE [[nodiscard]] float getRelativeLevel() const;

  Q_INVOKABLE [[nodiscard]] float getRangeLevel() const;

  Q_INVOKABLE [[nodiscard]] float getLoudnessLevel() const;

  Q_INVOKABLE [[nodiscard]] float getOutputGainLevel() const;

  Q_INVOKABLE void resetHistory();

 private:
  bool ebur128_ready = false;

  uint old_rate = 0U;

  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;
  double range = 0.0;
  double loudness = 0.0;
  double internal_output_gain = 1.0;
  double prev_gain = 1.0;
  double block_time = 0.0;
  double attack_time = 0.1;   // seconds
  double release_time = 0.4;  // seconds
  double attack_coeff = 1.0F;
  double release_coeff = 1.0F;

  std::vector<float> data;

  ebur128_state* ebur_state = nullptr;

  db::Autogain* settings = nullptr;

  auto init_ebur128() -> bool;

  void set_maximum_history(const int& seconds);
};
