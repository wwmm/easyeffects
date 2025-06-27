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

#include <pipewire/proxy.h>
#include <qtmetamacros.h>
#include <sys/types.h>
#include <QString>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_multiband_compressor.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class MultibandCompressor : public PluginBase {
  Q_OBJECT

 public:
  MultibandCompressor(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  MultibandCompressor(const MultibandCompressor&) = delete;
  auto operator=(const MultibandCompressor&) -> MultibandCompressor& = delete;
  MultibandCompressor(const MultibandCompressor&&) = delete;
  auto operator=(const MultibandCompressor&&) -> MultibandCompressor& = delete;
  ~MultibandCompressor() override;

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

  // auto get_latency_seconds() -> float override;

  // void update_probe_links() override;

  // TODO: add QT signals here
  // ...

 private:
  uint latency_n_frames = 0U;

  // TODO: add private parameters here
  // ...

  db::MultibandCompressor* settings = nullptr;

  std::vector<pw_proxy*> list_proxies;

  void update_sidechain_links();
};
