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

#ifndef STREAM_INPUT_EFFECTS_HPP
#define STREAM_INPUT_EFFECTS_HPP

#include "multiband_compressor.hpp"
#include "multiband_gate.hpp"
#include "pipe_manager.hpp"
#include "pipeline_base.hpp"
#include "webrtc.hpp"

class StreamInputEffects : public PipelineBase {
 public:
  StreamInputEffects(PipeManager* pipe_manager);
  StreamInputEffects(const StreamInputEffects&) = delete;
  auto operator=(const StreamInputEffects&) -> StreamInputEffects& = delete;
  StreamInputEffects(const StreamInputEffects&&) = delete;
  auto operator=(const StreamInputEffects&&) -> StreamInputEffects& = delete;
  ~StreamInputEffects() override;

  std::unique_ptr<Webrtc> webrtc;
  std::unique_ptr<MultibandCompressor> multiband_compressor;
  std::unique_ptr<MultibandGate> multiband_gate;

  void change_input_device(const NodeInfo& node);

  sigc::signal<void(std::array<double, 2>)> webrtc_input_level;
  sigc::signal<void(std::array<double, 2>)> webrtc_output_level;

 private:
  void add_plugins_to_pipeline();

  /*
    Do not pass nd_info by reference. Sometimes it dies before we use it and a segmentation fault happens
  */

  void on_app_added(NodeInfo node_info);

  void on_link_changed(LinkInfo link_info);

  void on_source_changed(NodeInfo node_info);
};

#endif
