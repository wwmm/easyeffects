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

#include <cstdint>

#include <qobject.h>
#include <qqmlintegration.h>
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
  QML_NAMED_ELEMENT(BackendDeepFilterNet)
  QML_UNCREATABLE("Use the c++ instance")

  Q_PROPERTY(bool deepFilterNetInstalled MEMBER deepFilterNetInstalled CONSTANT)
  Q_PROPERTY(bool dpdfnetInstalled MEMBER dpdfnetInstalled CONSTANT)
  Q_PROPERTY(int npuPolicy READ npuPolicy NOTIFY npuChanged)
  Q_PROPERTY(bool npuAvailable READ npuAvailable NOTIFY npuChanged)
  Q_PROPERTY(bool npuPolicyAvailable READ npuPolicyAvailable NOTIFY npuChanged)
  Q_PROPERTY(bool npuPolicyWriteFailed READ npuPolicyWriteFailed NOTIFY npuChanged)

 public:
  DeepFilterNet(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  DeepFilterNet(const DeepFilterNet&) = delete;
  auto operator=(const DeepFilterNet&) -> DeepFilterNet& = delete;
  DeepFilterNet(const DeepFilterNet&&) = delete;
  auto operator=(const DeepFilterNet&&) -> DeepFilterNet& = delete;
  ~DeepFilterNet() override;

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

  Q_INVOKABLE void resetHistory();
  Q_INVOKABLE void refreshNpuPolicy();
  Q_INVOKABLE void setNpuPolicy(int policy);
  [[nodiscard]] auto npuPolicy() const -> int;
  [[nodiscard]] auto npuAvailable() const -> bool;
  [[nodiscard]] auto npuPolicyAvailable() const -> bool { return npu_policy >= 0; }
  [[nodiscard]] auto npuPolicyWriteFailed() const -> bool { return npu_policy_write_failed; }

 Q_SIGNALS:
  void npuChanged();

 private:
  DbDeepFilterNet* settings = nullptr;

  std::unique_ptr<ladspa::LadspaWrapper> ladspa_wrapper;

  // DPDFNet is mono only, so stereo runs one instance per channel
  std::unique_ptr<ladspa::LadspaWrapper> dpdfnet_l, dpdfnet_r;

  using NpuPrepare = std::uint64_t (*)(bool);
  using NpuGetPolicy = std::int32_t (*)();
  using NpuSetPolicy = std::int32_t (*)(std::uint8_t);
  NpuPrepare npu_prepare = nullptr;
  NpuGetPolicy npu_get_policy = nullptr;
  NpuSetPolicy npu_set_policy = nullptr;
  bool npu_query_pending = false;
  bool npu_policy_write_failed = false;
  int npu_queued_policy = -1;
  int npu_policy = -1;
  void updateNpuPolicy(int requested);

  bool deepFilterNetInstalled = false;
  bool dpdfnetInstalled = false;
  bool use_dpdfnet = false;
  bool mono_processing = false;
  std::vector<float> mono_input;

  bool ready = false;
  std::uint64_t setup_generation = 0;
  bool resample = false;
  bool resampler_ready = true;

  std::unique_ptr<Resampler> resampler_inL, resampler_outL;
  std::unique_ptr<Resampler> resampler_inR, resampler_outR;

  std::vector<float> resampled_outL, resampled_outR;
  std::vector<float> carryover_l, carryover_r;

  void run_model(std::span<const float> left_in,
                 std::span<const float> right_in,
                 std::span<float> left_out,
                 std::span<float> right_out);
};
