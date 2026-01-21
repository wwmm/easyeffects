/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects
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

#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <sys/types.h>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_equalizer.h"
#include "easyeffects_db_equalizer_channel.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"

class Equalizer : public PluginBase {
  Q_OBJECT

 public:
  Equalizer(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  Equalizer(const Equalizer&) = delete;
  auto operator=(const Equalizer&) -> Equalizer& = delete;
  Equalizer(const Equalizer&&) = delete;
  auto operator=(const Equalizer&&) -> Equalizer& = delete;
  ~Equalizer() override;

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

  Q_INVOKABLE void sortBands();

  Q_INVOKABLE void flatResponse();

  Q_INVOKABLE void calculateFrequencies();

  Q_INVOKABLE bool importApoPreset(const QList<QString>& url_list);

  Q_INVOKABLE bool importApoGraphicEqPreset(const QList<QString>& url_list);

  Q_INVOKABLE bool exportApoPreset(const QString& url);

  static constexpr int max_bands = 32;

 private:
  db::Equalizer* settings = nullptr;
  db::EqualizerChannel *settings_left = nullptr, *settings_right = nullptr;

  bool ready = false;

  uint latency_n_frames = 0U;

  std::vector<QMetaObject::Connection> unified_mode_connections;

  void bind_left_bands();

  void bind_right_bands();

  void on_split_channels();
};
