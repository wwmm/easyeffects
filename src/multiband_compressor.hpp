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
#include <qlist.h>
#include <qqmlintegration.h>
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
#include "tags_multiband_compressor.hpp"

class MultibandCompressor : public PluginBase {
  Q_OBJECT
  QML_NAMED_ELEMENT(BackendMultibandCompressor)
  QML_UNCREATABLE("Use the c++ instance")

 public:
  MultibandCompressor(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  MultibandCompressor(const MultibandCompressor&) = delete;
  auto operator=(const MultibandCompressor&) -> MultibandCompressor& = delete;
  MultibandCompressor(const MultibandCompressor&&) = delete;
  auto operator=(const MultibandCompressor&&) -> MultibandCompressor& = delete;
  ~MultibandCompressor() override;

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

  void update_probe_links() override;

  Q_INVOKABLE [[nodiscard]] QList<double> getFrequencyRangeEnd() const;

  Q_INVOKABLE [[nodiscard]] QList<double> getEnvelopeLevelLeft() const;
  Q_INVOKABLE [[nodiscard]] QList<double> getEnvelopeLevelRight() const;

  Q_INVOKABLE [[nodiscard]] QList<double> getCurveLevelLeft() const;
  Q_INVOKABLE [[nodiscard]] QList<double> getCurveLevelRight() const;

  Q_INVOKABLE [[nodiscard]] QList<double> getReductionLevelLeft() const;
  Q_INVOKABLE [[nodiscard]] QList<double> getReductionLevelRight() const;

 private:
  uint latency_n_frames = 0U;

  bool ready = false;

  static constexpr uint n_bands = tags::multiband_compressor::n_bands;

  DbMultibandCompressor* settings = nullptr;

  // Using double instead of float helps qml replace JS by c++ calls in the plugin updateMeters() function
  QList<double> frequency_range_end, envelope_left, envelope_right, curve_left, curve_right, reduction_left,
      reduction_right;

  std::vector<pw_proxy*> list_proxies;

  void update_sidechain_links();

  void bind_bands();
};
