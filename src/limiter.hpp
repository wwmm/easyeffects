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

#include <pipewire/proxy.h>
#include <qtmetamacros.h>
#include <sys/types.h>
#include <QString>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_limiter.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Limiter : public PluginBase {
  Q_OBJECT

 public:
  Limiter(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Limiter(const Limiter&) = delete;
  auto operator=(const Limiter&) -> Limiter& = delete;
  Limiter(const Limiter&&) = delete;
  auto operator=(const Limiter&&) -> Limiter& = delete;
  ~Limiter() override;

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

  void update_probe_links() override;

  auto get_latency_seconds() -> float override;

  Q_INVOKABLE [[nodiscard]] float getGainLevelLeft() const;
  Q_INVOKABLE [[nodiscard]] float getGainLevelRight() const;
  Q_INVOKABLE [[nodiscard]] float getSideChainLevelLeft() const;
  Q_INVOKABLE [[nodiscard]] float getSideChainLevelRight() const;

 private:
  uint latency_n_frames = 0U;

  float gain_l_port_value = 0.0F;
  float gain_r_port_value = 0.0F;
  float sidechain_l_port_value = 0.0F;
  float sidechain_r_port_value = 0.0F;

  bool ready = false;

  db::Limiter* settings = nullptr;

  std::vector<pw_proxy*> list_proxies;

  void update_sidechain_links();
};
