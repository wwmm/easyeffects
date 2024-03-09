/*
 *  Copyright © 2017-2024 Wellington Wallace
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
#include <pipewire/context.h>
#include <pipewire/proxy.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "autogain.hpp"              // IWYU pragma: export
#include "bass_enhancer.hpp"         // IWYU pragma: export
#include "bass_loudness.hpp"         // IWYU pragma: export
#include "compressor.hpp"            // IWYU pragma: export
#include "convolver.hpp"             // IWYU pragma: export
#include "crossfeed.hpp"             // IWYU pragma: export
#include "crystalizer.hpp"           // IWYU pragma: export
#include "deepfilternet.hpp"         // IWYU pragma: export
#include "deesser.hpp"               // IWYU pragma: export
#include "delay.hpp"                 // IWYU pragma: export
#include "echo_canceller.hpp"        // IWYU pragma: export
#include "equalizer.hpp"             // IWYU pragma: export
#include "exciter.hpp"               // IWYU pragma: export
#include "expander.hpp"              // IWYU pragma: export
#include "filter.hpp"                // IWYU pragma: export
#include "gate.hpp"                  // IWYU pragma: export
#include "limiter.hpp"               // IWYU pragma: export
#include "loudness.hpp"              // IWYU pragma: export
#include "maximizer.hpp"             // IWYU pragma: export
#include "multiband_compressor.hpp"  // IWYU pragma: export
#include "multiband_gate.hpp"        // IWYU pragma: export
#include "output_level.hpp"          // IWYU pragma: export
#include "pipe_manager.hpp"          // IWYU pragma: export
#include "pitch.hpp"                 // IWYU pragma: export
#include "plugin_base.hpp"           // IWYU pragma: export
#include "reverb.hpp"                // IWYU pragma: export
#include "rnnoise.hpp"               // IWYU pragma: export
#include "spectrum.hpp"              // IWYU pragma: export
#include "speex.hpp"                 // IWYU pragma: export
#include "stereo_tools.hpp"          // IWYU pragma: export
#include "tags_resources.hpp"        // IWYU pragma: export
#include "tags_schema.hpp"           // IWYU pragma: export

class EffectsBase {
 public:
  EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager);
  EffectsBase(const EffectsBase&) = delete;
  auto operator=(const EffectsBase&) -> EffectsBase& = delete;
  EffectsBase(const EffectsBase&&) = delete;
  auto operator=(const EffectsBase&&) -> EffectsBase& = delete;
  virtual ~EffectsBase();

  const std::string log_tag;

  PipeManager* pm = nullptr;

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
