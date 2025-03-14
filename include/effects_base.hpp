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

#include <gio/gio.h>
#include <glib.h>
#include <pipewire/proxy.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "autogain.hpp"
#include "bass_enhancer.hpp"
#include "bass_loudness.hpp"
#include "compressor.hpp"
#include "convolver.hpp"
#include "crossfeed.hpp"
#include "crystalizer.hpp"
#include "deepfilternet.hpp"
#include "deesser.hpp"
#include "delay.hpp"
#include "echo_canceller.hpp"
#include "equalizer.hpp"
#include "exciter.hpp"
#include "expander.hpp"
#include "filter.hpp"
#include "gate.hpp"
#include "limiter.hpp"
#include "loudness.hpp"
#include "maximizer.hpp"
#include "multiband_compressor.hpp"
#include "multiband_gate.hpp"
#include "output_level.hpp"
#include "pipe_manager.hpp"
#include "pitch.hpp"
#include "plugin_base.hpp"
#include "reverb.hpp"
#include "rnnoise.hpp"
#include "spectrum.hpp"
#include "speex.hpp"
#include "stereo_tools.hpp"

class EffectsBase {
 public:
  EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager, PipelineType pipe_type);
  EffectsBase(const EffectsBase&) = delete;
  auto operator=(const EffectsBase&) -> EffectsBase& = delete;
  EffectsBase(const EffectsBase&&) = delete;
  auto operator=(const EffectsBase&&) -> EffectsBase& = delete;
  virtual ~EffectsBase();

  const std::string log_tag;

  PipeManager* pm = nullptr;

  PipelineType pipeline_type;

  std::shared_ptr<OutputLevel> output_level;
  std::shared_ptr<Spectrum> spectrum;

  std::shared_ptr<AutoGain> autogain;
  std::shared_ptr<BassEnhancer> bass_enhancer;
  std::shared_ptr<BassLoudness> bass_loudness;
  std::shared_ptr<Compressor> compressor;
  std::shared_ptr<Convolver> convolver;
  std::shared_ptr<Crossfeed> crossfeed;
  std::shared_ptr<Crystalizer> crystalizer;
  std::shared_ptr<DeepFilterNet> deepfilternet;
  std::shared_ptr<Deesser> deesser;
  std::shared_ptr<Delay> delay;
  std::shared_ptr<EchoCanceller> echo_canceller;
  std::shared_ptr<Equalizer> equalizer;
  std::shared_ptr<Exciter> exciter;
  std::shared_ptr<Expander> expander;
  std::shared_ptr<Filter> filter;
  std::shared_ptr<Gate> gate;
  std::shared_ptr<Limiter> limiter;
  std::shared_ptr<Loudness> loudness;
  std::shared_ptr<Maximizer> maximizer;
  std::shared_ptr<MultibandCompressor> multiband_compressor;
  std::shared_ptr<MultibandGate> multiband_gate;
  std::shared_ptr<Pitch> pitch;
  std::shared_ptr<Reverb> reverb;
  std::shared_ptr<RNNoise> rnnoise;
  std::shared_ptr<Speex> speex;
  std::shared_ptr<StereoTools> stereo_tools;

  auto get_pipeline_latency() -> float;

  void reset_settings();

  sigc::signal<void(const float&)> pipeline_latency;

  auto get_plugins_map() -> std::map<std::string, std::shared_ptr<PluginBase>>;

  template <typename T>
  auto get_plugin_instance(const std::string& name) -> std::shared_ptr<T> {
    return std::dynamic_pointer_cast<T>(plugins[name]);
  }

 protected:
  GSettings *settings = nullptr, *global_settings = nullptr;

  std::string schema_base_path;

  std::map<std::string, std::shared_ptr<PluginBase>> plugins;

  std::vector<pw_proxy*> list_proxies, list_proxies_listen_mic;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections, gconnections_global;

  void create_filters_if_necessary();

  void remove_unused_filters();

  void activate_filters();

  void deactivate_filters();

  void broadcast_pipeline_latency();
};
