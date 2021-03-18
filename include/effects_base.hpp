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

#ifndef EFFECTS_BASE_HPP
#define EFFECTS_BASE_HPP

#include <giomm.h>
#include "autogain.hpp"
#include "bass_enhancer.hpp"
#include "compressor.hpp"
#include "convolver.hpp"
#include "crossfeed.hpp"
#include "crystalizer.hpp"
#include "deesser.hpp"
#include "delay.hpp"
#include "equalizer.hpp"
#include "exciter.hpp"
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
#include "reverb.hpp"
#include "rnnoise.hpp"
#include "spectrum.hpp"
#include "stereo_tools.hpp"

class EffectsBase {
 public:
  EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager);
  EffectsBase(const EffectsBase&) = delete;
  auto operator=(const EffectsBase&) -> EffectsBase& = delete;
  EffectsBase(const EffectsBase&&) = delete;
  auto operator=(const EffectsBase&&) -> EffectsBase& = delete;
  virtual ~EffectsBase();

  std::string log_tag;

  PipeManager* pm = nullptr;

  std::unique_ptr<OutputLevel> output_level;
  std::unique_ptr<Spectrum> spectrum;

  std::shared_ptr<AutoGain> autogain;
  std::shared_ptr<BassEnhancer> bass_enhancer;
  std::shared_ptr<Exciter> exciter;
  std::shared_ptr<Filter> filter;
  std::shared_ptr<Limiter> limiter;
  std::shared_ptr<Maximizer> maximizer;
  std::shared_ptr<Reverb> reverb;
  std::shared_ptr<StereoTools> stereo_tools;

 protected:
  Glib::RefPtr<Gio::Settings> settings, global_settings;

  bool apps_want_to_play = false;

  std::map<std::string, std::shared_ptr<PluginBase>> plugins;

  std::shared_ptr<Compressor> compressor;
  std::shared_ptr<Convolver> convolver;
  std::shared_ptr<Crossfeed> crossfeed;
  std::shared_ptr<Crystalizer> crystalizer;
  std::shared_ptr<Deesser> deesser;
  std::shared_ptr<Delay> delay;
  std::shared_ptr<Equalizer> equalizer;
  std::shared_ptr<Gate> gate;
  std::shared_ptr<Loudness> loudness;
  std::shared_ptr<MultibandCompressor> multiband_compressor;
  std::shared_ptr<MultibandGate> multiband_gate;
  std::shared_ptr<Pitch> pitch;
  std::shared_ptr<RNNoise> rnnoise;

  void activate_filters();

  void deactivate_filters();
};

#endif