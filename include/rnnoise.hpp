/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RNNOISE_HPP
#define RNNOISE_HPP

#include <rnnoise.h>
#include <memory>
#include "plugin_base.hpp"
#include "resampler.hpp"

class RNNoise : public PluginBase {
 public:
  RNNoise(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
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

  sigc::signal<void(const float&)> latency;

 private:
  bool resample = false;
  bool notify_latency = false;
  bool rnnoise_ready = false;
  bool resampler_ready = false;

  uint blocksize = 480U;
  uint rnnoise_rate = 48000U;
  uint latency_n_frames = 0U;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1);

  std::deque<float> deque_out_L, deque_out_R;

  std::vector<float> data_L, data_R;
  std::vector<float> resampled_data_L, resampled_data_R;

  std::unique_ptr<Resampler> resampler_inL, resampler_outL;
  std::unique_ptr<Resampler> resampler_inR, resampler_outR;

  RNNModel* model = nullptr;

  DenoiseState *state_left = nullptr, *state_right = nullptr;

  auto get_model_from_file() -> RNNModel*;

  void free_rnnoise();

  template <typename T1, typename T2>
  void remove_noise(const T1& left_in, const T1& right_in, T2& out_L, T2& out_R) {
    for (const auto& v : left_in) {
      data_L.emplace_back(v);

      if (data_L.size() == blocksize) {
        if (state_left != nullptr) {
          std::ranges::for_each(data_L, [](auto& v) { v *= static_cast<float>(SHRT_MAX + 1); });

          rnnoise_process_frame(state_left, data_L.data(), data_L.data());

          std::ranges::for_each(data_L, [&](auto& v) { v *= inv_short_max; });
        }

        for (const auto& v : data_L) {
          out_L.emplace_back(v);
        }

        data_L.resize(0);
      }
    }

    for (const auto& v : right_in) {
      data_R.emplace_back(v);

      if (data_R.size() == blocksize) {
        if (state_right != nullptr) {
          std::ranges::for_each(data_R, [](auto& v) { v *= static_cast<float>(SHRT_MAX + 1); });

          rnnoise_process_frame(state_right, data_R.data(), data_R.data());

          std::ranges::for_each(data_R, [&](auto& v) { v *= inv_short_max; });
        }

        for (const auto& v : data_R) {
          out_R.emplace_back(v);
        }

        data_R.resize(0);
      }
    }
  }
};

#endif
