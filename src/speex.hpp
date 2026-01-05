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

#include <qtmetamacros.h>
#include <speex/speex_preprocess.h>
#include <speex/speexdsp_config_types.h>
#include <sys/types.h>
#include <QString>
#include <climits>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_speex.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Speex : public PluginBase {
  Q_OBJECT

 public:
  Speex(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Speex(const Speex&) = delete;
  auto operator=(const Speex&) -> Speex& = delete;
  Speex(const Speex&&) = delete;
  auto operator=(const Speex&&) -> Speex& = delete;
  ~Speex() override;

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
  db::Speex* settings = nullptr;

  bool speex_ready = false;

  int enable_denoise = 0, noise_suppression = -15, enable_agc = 0, enable_vad = 0, vad_probability_start = 95,
      vad_probability_continue = 90, enable_dereverb = 0;

  uint latency_n_frames = 0U;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1);

  std::vector<spx_int16_t> data_L, data_R;

  SpeexPreprocessState *state_left = nullptr, *state_right = nullptr;

  void free_speex();
};
