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

#include <rubberband/RubberBandStretcher.h>
#include <deque>
#include "plugin_base.hpp"

class Pitch : public PluginBase {
 public:
  Pitch(const std::string& tag, const std::string& schema, const std::string& schema_path, PipeManager* pipe_manager);
  Pitch(const Pitch&) = delete;
  auto operator=(const Pitch&) -> Pitch& = delete;
  Pitch(const Pitch&&) = delete;
  auto operator=(const Pitch&&) -> Pitch& = delete;
  ~Pitch() override;

  enum class Mode { speed, quality, consistency };
  enum class Formant { shifted, preserved };
  enum class Transients { crisp, mixed, smooth };
  enum class Detector { compound, percussive, soft };
  enum class Phase { laminar, independent };

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  auto get_latency_seconds() -> float override;

  float latency_value = 0.0F;

 private:
  bool rubberband_ready = false;
  bool notify_latency = false;

  uint latency_n_frames = 0U;

  std::vector<float> data_L, data_R;

  std::array<float*, 2U> stretcher_in = {nullptr, nullptr};
  std::array<float*, 2U> stretcher_out = {nullptr, nullptr};

  std::deque<float> deque_out_L, deque_out_R;

  RubberBand::RubberBandStretcher* stretcher = nullptr;

  Mode mode = Mode::speed;
  Formant formant = Formant::shifted;
  Transients transients = Transients::crisp;
  Detector detector = Detector::compound;
  Phase phase = Phase::laminar;

  int cents = 0;
  int semitones = 0;
  int octaves = 0;

  double time_ratio = 1.0;

  void init_stretcher();

  static auto parse_mode_key(const std::string& key) -> Mode;
  static auto parse_formant_key(const std::string& key) -> Formant;
  static auto parse_transients_key(const std::string& key) -> Transients;
  static auto parse_detector_key(const std::string& key) -> Detector;
  static auto parse_phase_key(const std::string& key) -> Phase;

  void set_mode();
  void set_formant();
  void set_transients();
  void set_detector();
  void set_phase();
  void set_pitch_scale();
};
