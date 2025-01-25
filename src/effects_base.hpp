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

#include <kconfigskeleton.h>
#include <pipewire/proxy.h>
#include <qlist.h>
#include <qobject.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <QString>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "autogain.hpp"
#include "bass_enhancer.hpp"
#include "compressor.hpp"
#include "convolver.hpp"
#include "crossfeed.hpp"
#include "crystalizer.hpp"
#include "exciter.hpp"
#include "filter.hpp"
#include "gate.hpp"
#include "limiter.hpp"
#include "maximizer.hpp"
#include "output_level.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "spectrum.hpp"
#include "speex.hpp"
#include "stereo_tools.hpp"

// #include "bass_loudness.hpp"
// #include "deepfilternet.hpp"
// #include "deesser.hpp"
// #include "delay.hpp"
// #include "echo_canceller.hpp"
// #include "equalizer.hpp"
// #include "expander.hpp"
// #include "loudness.hpp"
// #include "multiband_compressor.hpp"
// #include "multiband_gate.hpp"
// #include "pitch.hpp"
// #include "reverb.hpp"
// #include "rnnoise.hpp"

class EffectsBase : public QObject {
  Q_OBJECT

 public:
  EffectsBase(pw::Manager* pipe_manager, PipelineType pipe_type);
  EffectsBase(const EffectsBase&) = delete;
  auto operator=(const EffectsBase&) -> EffectsBase& = delete;
  EffectsBase(const EffectsBase&&) = delete;
  auto operator=(const EffectsBase&&) -> EffectsBase& = delete;
  ~EffectsBase() override;

  const std::string log_tag;

  pw::Manager* pm = nullptr;

  PipelineType pipeline_type;

  std::shared_ptr<OutputLevel> output_level;
  std::shared_ptr<Spectrum> spectrum;

  std::shared_ptr<Autogain> autogain;
  std::shared_ptr<BassEnhancer> bass_enhancer;
  std::shared_ptr<Compressor> compressor;
  std::shared_ptr<Convolver> convolver;
  std::shared_ptr<Crossfeed> crossfeed;
  std::shared_ptr<Crystalizer> crystalizer;
  std::shared_ptr<Exciter> exciter;
  std::shared_ptr<Filter> filter;
  std::shared_ptr<Gate> gate;
  std::shared_ptr<Limiter> limiter;
  std::shared_ptr<Maximizer> maximizer;
  std::shared_ptr<Speex> speex;
  std::shared_ptr<StereoTools> stereo_tools;
  //   std::shared_ptr<BassLoudness> bass_loudness;
  //   std::shared_ptr<DeepFilterNet> deepfilternet;
  //   std::shared_ptr<Deesser> deesser;
  //   std::shared_ptr<Delay> delay;
  //   std::shared_ptr<EchoCanceller> echo_canceller;
  //   std::shared_ptr<Equalizer> equalizer;
  //   std::shared_ptr<Expander> expander;
  //   std::shared_ptr<Loudness> loudness;
  //   std::shared_ptr<MultibandCompressor> multiband_compressor;
  //   std::shared_ptr<MultibandGate> multiband_gate;
  //   std::shared_ptr<Pitch> pitch;
  //   std::shared_ptr<Reverb> reverb;
  //   std::shared_ptr<RNNoise> rnnoise;

  auto get_plugins_map() -> std::map<QString, std::shared_ptr<PluginBase>>;

  Q_INVOKABLE QVariant getPluginInstance(const QString& pluginName);

  Q_INVOKABLE [[nodiscard]] uint getPipeLineRate() const;

  Q_INVOKABLE [[nodiscard]] uint getPipeLineLatency();

  Q_INVOKABLE [[nodiscard]] float getOutputLevelLeft() const;

  Q_INVOKABLE [[nodiscard]] float getOutputLevelRight() const;

  Q_INVOKABLE void requestSpectrumData();

 signals:
  void pipelineChanged();
  void newSpectrumData(QList<QPointF> newData);

 protected:
  std::map<QString, std::shared_ptr<PluginBase>> plugins;

  std::vector<pw_proxy*> list_proxies, list_proxies_listen_mic;

  void create_filters_if_necessary();

  void remove_unused_filters();

  void activate_filters();

  void deactivate_filters();
};
