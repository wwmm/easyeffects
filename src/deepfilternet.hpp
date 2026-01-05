/**
 * Copyright © 2023-2026 Torge Matthies
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

#include <qobject.h>
#include <qtmetamacros.h>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_deepfilternet.h"
#include "ladspa_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "resampler.hpp"

class DeepFilterNet : public PluginBase {
  Q_OBJECT

 public:
  DeepFilterNet(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  DeepFilterNet(const DeepFilterNet&) = delete;
  auto operator=(const DeepFilterNet&) -> DeepFilterNet& = delete;
  DeepFilterNet(const DeepFilterNet&&) = delete;
  auto operator=(const DeepFilterNet&&) -> DeepFilterNet& = delete;
  ~DeepFilterNet() override;

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

  Q_INVOKABLE void resetHistory();

 private:
  db::DeepFilterNet* settings = nullptr;

  std::unique_ptr<ladspa::LadspaWrapper> ladspa_wrapper;

  bool ready = false;
  bool resample = false;
  bool resampler_ready = true;

  std::unique_ptr<Resampler> resampler_inL, resampler_outL;
  std::unique_ptr<Resampler> resampler_inR, resampler_outR;

  std::vector<float> resampled_outL, resampled_outR;
  std::vector<float> carryover_l, carryover_r;
};
