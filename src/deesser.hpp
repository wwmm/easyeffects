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

#include <qobject.h>
#include <qtmetamacros.h>
#include <span>
#include <string>
#include "easyeffects_db_deesser.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Deesser : public PluginBase {
  Q_OBJECT

 public:
  Deesser(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Deesser(const Deesser&) = delete;
  auto operator=(const Deesser&) -> Deesser& = delete;
  Deesser(const Deesser&&) = delete;
  auto operator=(const Deesser&&) -> Deesser& = delete;
  ~Deesser() override;

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

  Q_INVOKABLE [[nodiscard]] float getCompressionLevel() const;
  Q_INVOKABLE [[nodiscard]] float getDetectedLevel() const;

 private:
  db::Deesser* settings = nullptr;

  bool ready = false;

  float compression_value = 0.0F;
  float detected_value = 0.0F;
};
