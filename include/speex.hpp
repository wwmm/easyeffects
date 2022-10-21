/*
 *  Copyright © 2017-2023 Wellington Wallace
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

#ifdef SPEEX_AVAILABLE
#include <speex/speex_preprocess.h>
#endif

#include <deque>
#include "plugin_base.hpp"
class Speex : public PluginBase {
 public:
  Speex(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
  Speex(const Speex&) = delete;
  auto operator=(const Speex&) -> Speex& = delete;
  Speex(const Speex&&) = delete;
  auto operator=(const Speex&&) -> Speex& = delete;
  ~Speex() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  auto get_latency_seconds() -> float override;

#ifndef SPEEX_AVAILABLE
  bool package_installed = false;
#endif

  float latency_value = 0.0F;

 private:
  bool notify_latency = false;

  uint blocksize = 480U;
  uint latency_n_frames = 0U;

  const float inv_short_max = 1.0F / (SHRT_MAX + 1);

  std::deque<float> deque_out_L, deque_out_R;

  std::vector<short> data_L, data_R; // Speex requires the data to be in 16-bit integers.

#ifdef SPEEX_AVAILABLE

  SpeexPreprocessState *state_left = nullptr, *state_right = nullptr;

  void free_speex();

  template <typename T1, typename T2>
  void remove_noise(const T1& left_in, const T1& right_in, T2& out_L, T2& out_R) {
    for (const auto& v : left_in) {
      data_L.push_back(v * static_cast<float>(SHRT_MAX + 1));

      if (data_L.size() == blocksize) {
        if (state_left != nullptr) {
          speex_preprocess_run(state_left, data_L.data());
        }

        for (const auto& v : data_L) {
          out_L.push_back(static_cast<float>(v) * inv_short_max);
        }

        data_L.resize(0U);
      }
    }

    for (const auto& v : right_in) {
      data_R.push_back(v * static_cast<float>(SHRT_MAX + 1));

      if (data_R.size() == blocksize) {
        if (state_right != nullptr) {
          speex_preprocess_run(state_right, data_R.data());
        }

        for (const auto& v : data_R) {
          out_R.push_back(static_cast<float>(v) * inv_short_max);
        }

        data_R.resize(0U);
      }
    }
  }

#endif
};
