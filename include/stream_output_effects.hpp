/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STREAM_OUTPUT_EFFECTS_HPP
#define STREAM_OUTPUT_EFFECTS_HPP

#include "autogain.hpp"
#include "bass_enhancer.hpp"
#include "convolver.hpp"
#include "crossfeed.hpp"
#include "crystalizer.hpp"
#include "delay.hpp"
#include "exciter.hpp"
#include "loudness.hpp"
#include "multiband_compressor.hpp"
#include "multiband_gate.hpp"
#include "pipe_manager.hpp"
#include "pipeline_base.hpp"

class StreamOutputEffects : public PipelineBase {
 public:
  StreamOutputEffects(PipeManager* pipe_manager);
  StreamOutputEffects(const StreamOutputEffects&) = delete;
  auto operator=(const StreamOutputEffects&) -> StreamOutputEffects& = delete;
  StreamOutputEffects(const StreamOutputEffects&&) = delete;
  auto operator=(const StreamOutputEffects&&) -> StreamOutputEffects& = delete;
  ~StreamOutputEffects() override;

  std::unique_ptr<BassEnhancer> bass_enhancer;
  std::unique_ptr<Exciter> exciter;
  std::unique_ptr<Crossfeed> crossfeed;
  std::unique_ptr<MultibandCompressor> multiband_compressor;
  std::unique_ptr<Loudness> loudness;
  std::unique_ptr<MultibandGate> multiband_gate;
  std::unique_ptr<Convolver> convolver;
  std::unique_ptr<Crystalizer> crystalizer;
  std::unique_ptr<AutoGain> autogain;
  std::unique_ptr<Delay> delay;

  void change_output_device(const NodeInfo& node);

  sigc::signal<void(std::array<double, 2>)> bass_enhancer_input_level;
  sigc::signal<void(std::array<double, 2>)> bass_enhancer_output_level;
  sigc::signal<void(std::array<double, 2>)> exciter_input_level;
  sigc::signal<void(std::array<double, 2>)> exciter_output_level;
  sigc::signal<void(std::array<double, 2>)> crossfeed_input_level;
  sigc::signal<void(std::array<double, 2>)> crossfeed_output_level;
  sigc::signal<void(std::array<double, 2>)> loudness_input_level;
  sigc::signal<void(std::array<double, 2>)> loudness_output_level;
  sigc::signal<void(std::array<double, 2>)> convolver_input_level;
  sigc::signal<void(std::array<double, 2>)> convolver_output_level;
  sigc::signal<void(std::array<double, 2>)> crystalizer_input_level;
  sigc::signal<void(std::array<double, 2>)> crystalizer_output_level;
  sigc::signal<void(std::array<double, 2>)> autogain_input_level;
  sigc::signal<void(std::array<double, 2>)> autogain_output_level;
  sigc::signal<void(std::array<double, 2>)> delay_input_level;
  sigc::signal<void(std::array<double, 2>)> delay_output_level;

 private:
  void add_plugins_to_pipeline();

  /*
    Do not pass nd_info by reference. Sometimes it dies before we use it and a segmentation fault happens
  */

  void on_app_added(NodeInfo node_info);

  void on_link_changed(LinkInfo link_info);

  void on_sink_changed(NodeInfo node_info);
};

#endif
