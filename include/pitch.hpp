/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#ifndef PITCH_HPP
#define PITCH_HPP

#include <rubberband/RubberBandStretcher.h>
#include "plugin_base.hpp"

class Pitch : public PluginBase {
 public:
  Pitch(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
  Pitch(const Pitch&) = delete;
  auto operator=(const Pitch&) -> Pitch& = delete;
  Pitch(const Pitch&&) = delete;
  auto operator=(const Pitch&&) -> Pitch& = delete;
  ~Pitch() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  sigc::signal<void(double)> latency;

 private:
  bool rubberband_ready = false;
  bool notify_latency = false;
  bool formant_preserving = false;
  bool faster = false;

  int crispness = 3;
  int cents = 0;
  int semitones = 0;
  int octaves = 0;

  uint latency_n_frames = 0U;

  double time_ratio = 1.0;

  std::vector<float> data_L, data_R;

  std::array<float*, 2> stretcher_in = {nullptr, nullptr};
  std::array<float*, 2> stretcher_out = {nullptr, nullptr};

  std::deque<float> deque_out_L, deque_out_R;

  RubberBand::RubberBandStretcher* stretcher = nullptr;

  void update_crispness();

  void update_pitch_scale();

  void init_stretcher();
};

#endif
