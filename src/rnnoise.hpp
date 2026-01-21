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
#include <sys/types.h>
#include <QString>
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include "easyeffects_db_rnnoise.h"
#include "pipeline_type.hpp"
#include "pw_manager.hpp"
#ifdef ENABLE_RNNOISE
#include <rnnoise.h>
#endif

#include "plugin_base.hpp"
#include "resampler.hpp"

class RNNoise : public PluginBase {
  Q_OBJECT

  Q_PROPERTY(bool usingStandardModel MEMBER standard_model NOTIFY usingStandardModelChanged)

 public:
  RNNoise(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id);
  RNNoise(const RNNoise&) = delete;
  auto operator=(const RNNoise&) -> RNNoise& = delete;
  RNNoise(const RNNoise&&) = delete;
  auto operator=(const RNNoise&&) -> RNNoise& = delete;
  ~RNNoise() override;

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

  void init_release();

  auto search_model_path(const std::string& name) -> std::string;

  const std::string rnnn_ext = ".rnnn";

  bool standard_model = true;

 Q_SIGNALS:
  void standardModelLoaded();
  void customModelLoaded(QString name, bool success);
  void usingStandardModelChanged();

 private:
  db::RNNoise* settings = nullptr;

  std::string app_data_dir;
  std::string local_dir_rnnoise;
  std::vector<std::string> system_data_dir_rnnoise;

  bool resample = false;
  bool notify_latency = false;
  bool rnnoise_ready = false;
  bool resampler_ready = false;

  uint blocksize = 480U;
  uint rnnoise_rate = 48000U;
  uint latency_n_frames = 0U;

  float wet_ratio = 1.0F;
  uint release = 2U;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1.0F);

  std::vector<float> buf_out_L, buf_out_R;

  std::vector<float> data_L, data_R, data_tmp;
  std::vector<float> resampled_data_L, resampled_data_R;

  std::unique_ptr<Resampler> resampler_inL, resampler_outL;
  std::unique_ptr<Resampler> resampler_inR, resampler_outR;

#ifdef ENABLE_RNNOISE

  FILE* model_file = nullptr;

  RNNModel* model = nullptr;

  DenoiseState *state_left = nullptr, *state_right = nullptr;

  float vad_prob_left, vad_prob_right;
  int vad_grace_left, vad_grace_right;

  auto get_model_from_name() -> RNNModel*;

  void prepare_model();

  void free_rnnoise();

  template <typename T1, typename T2>
  void remove_noise(const T1& left_in, const T1& right_in, T2& out_L, T2& out_R) {
    for (const auto& v : left_in) {
      data_L.push_back(v);

      if (data_L.size() == blocksize) {
        if (state_left != nullptr) {
          std::ranges::for_each(data_L, [](auto& v) { v *= static_cast<float>(SHRT_MAX + 1); });

          data_tmp = data_L;

          vad_prob_left = rnnoise_process_frame(state_left, data_L.data(), data_L.data());

          if (settings->enableVad()) {
            if (vad_prob_left >= (settings->vadThres() * 0.01F)) {
              vad_grace_left = release;
            }

            if (vad_grace_left >= 0) {
              --vad_grace_left;

              for (size_t i = 0U; i < data_L.size(); i++) {
                data_L[i] = (data_L[i] * wet_ratio) + (data_tmp[i] * (1.0F - wet_ratio));

                data_L[i] *= inv_short_max;
              }
            } else {
              std::ranges::for_each(data_L, [&](auto& v) { v = 0.0F; });
            }
          } else {
            for (size_t i = 0U; i < data_L.size(); i++) {
              data_L[i] = (data_L[i] * wet_ratio) + (data_tmp[i] * (1.0F - wet_ratio));

              data_L[i] *= inv_short_max;
            }
          }
        }

        out_L.insert(out_L.end(), data_L.begin(), data_L.end());

        data_L.resize(0U);
      }
    }

    for (const auto& v : right_in) {
      data_R.push_back(v);

      if (data_R.size() == blocksize) {
        if (state_right != nullptr) {
          std::ranges::for_each(data_R, [](auto& v) { v *= static_cast<float>(SHRT_MAX + 1); });

          data_tmp = data_R;

          vad_prob_right = rnnoise_process_frame(state_right, data_R.data(), data_R.data());

          if (settings->enableVad()) {
            if (vad_prob_right >= (settings->vadThres() * 0.01F)) {
              vad_grace_right = release;
            }

            if (vad_grace_right >= 0) {
              --vad_grace_right;

              for (size_t i = 0U; i < data_R.size(); i++) {
                data_R[i] = (data_R[i] * wet_ratio) + (data_tmp[i] * (1.0F - wet_ratio));

                data_R[i] *= inv_short_max;
              }
            } else {
              std::ranges::for_each(data_R, [&](auto& v) { v = 0.0F; });
            }
          } else {
            for (size_t i = 0U; i < data_R.size(); i++) {
              data_R[i] = (data_R[i] * wet_ratio) + (data_tmp[i] * (1.0F - wet_ratio));

              data_R[i] *= inv_short_max;
            }
          }
        }

        out_R.insert(out_R.end(), data_R.begin(), data_R.end());

        data_R.resize(0U);
      }
    }
  }

#endif
};
