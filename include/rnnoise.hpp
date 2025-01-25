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

#include <sigc++/signal.h>
#include <sys/types.h>
#include <algorithm>
#include <climits>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include "pipe_manager.hpp"
#ifdef ENABLE_RNNOISE
#include <rnnoise.h>
#endif

#include <deque>
#include "plugin_base.hpp"
#include "resampler.hpp"

class RNNoise : public PluginBase {
 public:
  RNNoise(const std::string& tag,
          const std::string& schema,
          const std::string& schema_path,
          PipeManager* pipe_manager,
          PipelineType pipe_type);
  RNNoise(const RNNoise&) = delete;
  auto operator=(const RNNoise&) -> RNNoise& = delete;
  RNNoise(const RNNoise&&) = delete;
  auto operator=(const RNNoise&&) -> RNNoise& = delete;
  ~RNNoise() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  auto get_latency_seconds() -> float override;

  void init_release();

  auto search_model_path(const std::string& name) -> std::string;

#ifndef ENABLE_RNNOISE
  bool package_installed = false;
#endif

  const std::string rnnn_ext = ".rnnn";

  bool standard_model = true;

  sigc::signal<void(const bool load_error)> model_changed;

 private:
  std::string local_dir_rnnoise;
  std::vector<std::string> system_data_dir_rnnoise;

  bool resample = false;
  bool notify_latency = false;
  bool rnnoise_ready = false;
  bool resampler_ready = false;
  bool enable_vad = false;

  uint blocksize = 480U;
  uint rnnoise_rate = 48000U;
  uint latency_n_frames = 0U;

  float vad_thres = 0.95F;
  float wet_ratio = 1.0F;
  uint release = 2U;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1.0F);

  std::deque<float> deque_out_L, deque_out_R;

  std::vector<float> data_L, data_R, data_tmp;
  std::vector<float> resampled_data_L, resampled_data_R;

  std::unique_ptr<Resampler> resampler_inL, resampler_outL;
  std::unique_ptr<Resampler> resampler_inR, resampler_outR;

#ifdef ENABLE_RNNOISE

  RNNModel* model = nullptr;

  DenoiseState *state_left = nullptr, *state_right = nullptr;

  float vad_prob_left, vad_prob_right;
  int vad_grace_left, vad_grace_right;

  auto get_model_from_name() -> RNNModel*;

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

          if (enable_vad) {
            if (vad_prob_left >= vad_thres) {
              vad_grace_left = release;
            }

            if (vad_grace_left >= 0) {
              --vad_grace_left;

              for (size_t i = 0U; i < data_L.size(); i++) {
                data_L[i] = data_L[i] * wet_ratio + data_tmp[i] * (1.0F - wet_ratio);

                data_L[i] *= inv_short_max;
              }
            } else {
              std::ranges::for_each(data_L, [&](auto& v) { v = 0.0F; });
            }
          } else {
            for (size_t i = 0U; i < data_L.size(); i++) {
              data_L[i] = data_L[i] * wet_ratio + data_tmp[i] * (1.0F - wet_ratio);

              data_L[i] *= inv_short_max;
            }
          }
        }

        for (const auto& v : data_L) {
          out_L.push_back(v);
        }

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

          if (enable_vad) {
            if (vad_prob_right >= vad_thres) {
              vad_grace_right = release;
            }

            if (vad_grace_right >= 0) {
              --vad_grace_right;

              for (size_t i = 0U; i < data_R.size(); i++) {
                data_R[i] = data_R[i] * wet_ratio + data_tmp[i] * (1.0F - wet_ratio);

                data_R[i] *= inv_short_max;
              }
            } else {
              std::ranges::for_each(data_R, [&](auto& v) { v = 0.0F; });
            }
          } else {
            for (size_t i = 0U; i < data_R.size(); i++) {
              data_R[i] = data_R[i] * wet_ratio + data_tmp[i] * (1.0F - wet_ratio);

              data_R[i] *= inv_short_max;
            }
          }
        }

        for (const auto& v : data_R) {
          out_R.push_back(v);
        }

        data_R.resize(0U);
      }
    }
  }

#endif
};
