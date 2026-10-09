/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "effects_base.hpp"
#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qpoint.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <spa/utils/defs.h>
#include <QSharedPointer>
#include <QString>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include "autogain.hpp"
#include "autotune.hpp"
#include "bass_enhancer.hpp"
#include "bass_loudness.hpp"
#include "compressor.hpp"
#include "convolver.hpp"
#include "crossfeed.hpp"
#include "crosstalk_canceller.hpp"
#include "crusher.hpp"
#include "crystalizer.hpp"
#include "db_manager.hpp"
#include "deepfilternet.hpp"
#include "deesser.hpp"
#include "delay.hpp"
#include "echo_canceller.hpp"
#include "equalizer.hpp"
#include "exciter.hpp"
#include "midside_equalizer.hpp"
#include "expander.hpp"
#include "filter.hpp"
#include "gate.hpp"
#include "level_meter.hpp"
#include "limiter.hpp"
#include "loudness.hpp"
#include "maximizer.hpp"
#include "multiband_compressor.hpp"
#include "multiband_gate.hpp"
#include "output_level.hpp"
#include "pipeline_type.hpp"
#include "pitch.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "reverb.hpp"
#include "rnnoise.hpp"
#include "spectrum.hpp"
#include "spectrum_dsp.hpp"
#include "speex.hpp"
#include "stereo_tools.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"
#include "voice_suppressor.hpp"

EffectsBase::EffectsBase(pw::Manager* pipe_manager, PipelineType pipe_type)
    : log_tag(pipe_type == PipelineType::output ? "soe: " : "sie: "),
      pm(pipe_manager),
      pipeline_type(pipe_type),
      baseWorker(new EffectsBaseWorker) {
  using namespace std::string_literals;

  output_level = std::make_shared<OutputLevel>(log_tag, pm, pipeline_type, "0");

  spectrum = std::make_shared<Spectrum>(log_tag, pm, pipeline_type, "0");

  if (!output_level->connected_to_pw) {
    output_level->connect_to_pw();
  }

  if (!spectrum->connected_to_pw) {
    spectrum->connect_to_pw();
  }

  create_filters_if_necessary();

  switch (pipeline_type) {
    case PipelineType::input:
      connect(DbStreamInputs::self(), &DbStreamInputs::pluginsChanged, [&]() { create_filters_if_necessary(); });
      break;
    case PipelineType::output:
      connect(DbStreamOutputs::self(), &DbStreamOutputs::pluginsChanged, [&]() { create_filters_if_necessary(); });
      break;
  }

  connect(DbMain::self(), &DbMain::lv2uiUpdateFrequencyChanged, [&]() {
    auto v = DbMain::lv2uiUpdateFrequency();

    for (auto& plugin : plugins | std::views::values) {
      plugin->set_native_ui_update_frequency(v);
    }
  });

  // worker thread for the native ui and maybe also other things

  baseWorker->moveToThread(&workerThread);

  workerThread.start();

  connect(&workerThread, &QThread::finished, baseWorker, &QObject::deleteLater);
}

EffectsBase::~EffectsBase() {
  workerThread.quit();
  workerThread.wait();

  util::debug("effects_base: destroyed");
}

void EffectsBase::create_filters_if_necessary() {
  auto list = (pipeline_type == PipelineType::output ? DbStreamOutputs::plugins() : DbStreamInputs::plugins());

  if (list.empty()) {
    return;
  }

  for (const auto& name : list) {
    if (plugins.contains(name)) {
      continue;
    }

    auto instance_id = tags::plugin_name::get_id(name);

    std::unique_ptr<PluginBase> filter = nullptr;

    if (name.startsWith(tags::plugin_name::BaseName::autogain)) {
      filter = std::make_unique<Autogain>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::autotune)) {
      filter = std::make_unique<Autotune>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
      filter = std::make_unique<BassEnhancer>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::bassLoudness)) {
      filter = std::make_unique<BassLoudness>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::compressor)) {
      filter = std::make_unique<Compressor>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::convolver)) {
      filter = std::make_unique<Convolver>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::crossfeed)) {
      filter = std::make_unique<Crossfeed>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::crusher)) {
      filter = std::make_unique<Crusher>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::crystalizer)) {
      filter = std::make_unique<Crystalizer>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::deepfilternet)) {
      filter = std::make_unique<DeepFilterNet>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::deesser)) {
      filter = std::make_unique<Deesser>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::delay)) {
      filter = std::make_unique<Delay>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
      filter = std::make_unique<EchoCanceller>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::exciter)) {
      filter = std::make_unique<Exciter>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::expander)) {
      filter = std::make_unique<Expander>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::equalizer)) {
      filter = std::make_unique<Equalizer>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::midsideEqualizer)) {
      filter = std::make_unique<MidSideEqualizer>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::filter)) {
      filter = std::make_unique<Filter>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::gate)) {
      filter = std::make_unique<Gate>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::voiceSuppressor)) {
      filter = std::make_unique<VoiceSuppressor>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::crosstalkCanceller)) {
      filter = std::make_unique<CrosstalkCanceller>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::levelMeter)) {
      filter = std::make_unique<LevelMeter>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::limiter)) {
      filter = std::make_unique<Limiter>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::loudness)) {
      filter = std::make_unique<Loudness>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::maximizer)) {
      filter = std::make_unique<Maximizer>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::multibandCompressor)) {
      filter = std::make_unique<MultibandCompressor>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::multibandGate)) {
      filter = std::make_unique<MultibandGate>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::pitch)) {
      filter = std::make_unique<Pitch>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::reverb)) {
      filter = std::make_unique<Reverb>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::rnnoise)) {
      filter = std::make_unique<RNNoise>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::speex)) {
      filter = std::make_unique<Speex>(log_tag, pm, pipeline_type, instance_id);

    } else if (name.startsWith(tags::plugin_name::BaseName::stereoTools)) {
      filter = std::make_unique<StereoTools>(log_tag, pm, pipeline_type, instance_id);
    }

    if (filter != nullptr) {
      /**
       * The filters inherit from QObject and we do not want QML to take
       * ownership of them. Double free may happen in this case when closing
       * the window or doing similar actions that trigger qml cleanup. The way
       * to avoid this is making sure that the objects managed by the C++
       * backend already have a parent by the time they are used on QML.
       */
      filter->setParent(this);
    }

    plugins.insert(std::make_pair(name, std::move(filter)));
  }
}

void EffectsBase::remove_unused_filters() {
  auto list = (pipeline_type == PipelineType::output ? DbStreamOutputs::plugins() : DbStreamInputs::plugins());

  if (list.empty()) {
    plugins.clear();

    return;
  }

  for (auto it = plugins.begin(); it != plugins.end();) {
    auto key = it->first;

    if (std::ranges::find(list, key) == list.end()) {
      auto plugin = std::move(it->second);  // Move the unique_ptr

      if (plugin == nullptr) {
        it = plugins.erase(it);

        continue;
      }

      plugin->bypass = true;

      if (plugin->connected_to_pw) {
        plugin->disconnect_from_pw();
      }

      it = plugins.erase(it);
    } else {
      it++;
    }
  }
}

void EffectsBase::activate_filters() {
  for (auto& plugin : plugins | std::views::values) {
    plugin->set_active(true);
  }
}

void EffectsBase::deactivate_filters() {
  for (auto& plugin : plugins | std::views::values) {
    plugin->set_active(false);
  }
}

auto EffectsBase::get_plugins_map() -> std::map<QString, std::unique_ptr<PluginBase>>& {
  return plugins;
}

QVariant EffectsBase::getPluginInstance(const QString& pluginName) {
  if (!plugins.contains(pluginName)) {
    return {};
  }

  auto& p = plugins[pluginName];

  if (pluginName.startsWith(tags::plugin_name::BaseName::autogain)) {
    return QVariant::fromValue(dynamic_cast<Autogain*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::autotune)) {
    return QVariant::fromValue(dynamic_cast<Autotune*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::bassEnhancer)) {
    return QVariant::fromValue(dynamic_cast<BassEnhancer*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::bassLoudness)) {
    return QVariant::fromValue(dynamic_cast<BassLoudness*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::compressor)) {
    return QVariant::fromValue(dynamic_cast<Compressor*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::convolver)) {
    return QVariant::fromValue(dynamic_cast<Convolver*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::crossfeed)) {
    return QVariant::fromValue(dynamic_cast<Crossfeed*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::crusher)) {
    return QVariant::fromValue(dynamic_cast<Crusher*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::crystalizer)) {
    return QVariant::fromValue(dynamic_cast<Crystalizer*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::delay)) {
    return QVariant::fromValue(dynamic_cast<Delay*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::deepfilternet)) {
    return QVariant::fromValue(dynamic_cast<DeepFilterNet*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::deesser)) {
    return QVariant::fromValue(dynamic_cast<Deesser*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::echoCanceller)) {
    return QVariant::fromValue(dynamic_cast<EchoCanceller*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::equalizer)) {
    return QVariant::fromValue(dynamic_cast<Equalizer*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::midsideEqualizer)) {
    return QVariant::fromValue(dynamic_cast<MidSideEqualizer*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::exciter)) {
    return QVariant::fromValue(dynamic_cast<Exciter*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::expander)) {
    return QVariant::fromValue(dynamic_cast<Expander*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::filter)) {
    return QVariant::fromValue(dynamic_cast<Filter*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::gate)) {
    return QVariant::fromValue(dynamic_cast<Gate*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::voiceSuppressor)) {
    return QVariant::fromValue(dynamic_cast<VoiceSuppressor*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::crosstalkCanceller)) {
    return QVariant::fromValue(dynamic_cast<CrosstalkCanceller*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::levelMeter)) {
    return QVariant::fromValue(dynamic_cast<LevelMeter*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::limiter)) {
    return QVariant::fromValue(dynamic_cast<Limiter*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::loudness)) {
    return QVariant::fromValue(dynamic_cast<Loudness*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::maximizer)) {
    return QVariant::fromValue(dynamic_cast<Maximizer*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::multibandCompressor)) {
    return QVariant::fromValue(dynamic_cast<MultibandCompressor*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::multibandGate)) {
    return QVariant::fromValue(dynamic_cast<MultibandGate*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::pitch)) {
    return QVariant::fromValue(dynamic_cast<Pitch*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::reverb)) {
    return QVariant::fromValue(dynamic_cast<Reverb*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::rnnoise)) {
    return QVariant::fromValue(dynamic_cast<RNNoise*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::speex)) {
    return QVariant::fromValue(dynamic_cast<Speex*>(p.get()));

  } else if (pluginName.startsWith(tags::plugin_name::BaseName::stereoTools)) {
    return QVariant::fromValue(dynamic_cast<StereoTools*>(p.get()));
  }

  return {};
}

double EffectsBase::getPipeLineRate() const {
  switch (pipeline_type) {
    case PipelineType::input:
      if (auto node = pm->model_nodes.get_node_by_name(DbStreamInputs::inputDevice()); node.serial != SPA_ID_INVALID) {
        return node.rate * 0.001;
      }
      return 0.0F;
    case PipelineType::output: {
      if (auto node = pm->model_nodes.get_node_by_name(DbStreamOutputs::outputDevice());
          node.serial != SPA_ID_INVALID) {
        return node.rate * 0.001;
      }
      return 0.0F;
    }
    default:
      return 0;
  }
}

uint EffectsBase::getPipeLineLatency() {
  auto list = (pipeline_type == PipelineType::output ? DbStreamOutputs::plugins() : DbStreamInputs::plugins());

  auto v = 0.0F;

  for (const auto& name : list) {
    if (plugins.contains(name) && plugins[name] != nullptr) {
      v += plugins[name]->get_latency_seconds();
    }
  }

  return v * 1000.0F;
}

float EffectsBase::getOutputLevelLeft() const {
  return output_level->output_peak_left;
}

float EffectsBase::getOutputLevelRight() const {
  return output_level->output_peak_right;
}

void EffectsBase::requestSpectrumData() {
  /**
   * Technically we can do the same as the other Q_INVOKABLE methods and run
   * the whole thing in the QML thread. But in this case we have some heavy
   * operations that need to be done. It is probably better to do them in the
   * main thread and deliver the spectrum list to QML through a signal.
   */

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (spectrum->rate == 0U) {
          return;
        }

        const auto nyquist = 0.5F * static_cast<float>(spectrum->rate);

        const auto min_freq = std::clamp(static_cast<float>(DbSpectrum::minimumFrequency()), 1.0F, nyquist);
        const auto max_freq = std::clamp(static_cast<float>(DbSpectrum::maximumFrequency()), 1.0F, nyquist);

        if (min_freq > (max_freq - 100.0F)) {
          return;
        }

        const int npoints = DbSpectrum::nPoints();
        const bool log_axis = DbSpectrum::logarithmicHorizontalAxis();

        const bool axis_settings_changed =
            (cached_spectrum_min_freq != min_freq || cached_spectrum_max_freq != max_freq ||
             cached_spectrum_npoints != npoints || cached_spectrum_log_axis != log_axis);

        if (axis_settings_changed) {
          if (log_axis) {
            cached_spectrum_x_axis = util::logspace<double>(min_freq, max_freq, npoints);
          } else {
            cached_spectrum_x_axis = util::linspace<double>(min_freq, max_freq, npoints);
          }

          cached_spectrum_band_edges = spectrum_dsp::band_edges(cached_spectrum_x_axis, log_axis);
          cached_spectrum_levels.resize(cached_spectrum_x_axis.size());

          cached_spectrum_min_freq = min_freq;
          cached_spectrum_max_freq = max_freq;
          cached_spectrum_npoints = npoints;
          cached_spectrum_log_axis = log_axis;
        }

        if (!spectrum->compute_band_levels(cached_spectrum_band_edges, cached_spectrum_levels)) {
          return;
        }

        const double min_level = DbSpectrum::minimumLevel();
        const double max_level = std::max(DbSpectrum::maximumLevel(), min_level + 1.0);
        const double slope = DbSpectrum::slope();

        for (size_t n = 0; n < cached_spectrum_levels.size(); n++) {
          // Display tilt in dB per octave around 1 kHz
          cached_spectrum_levels[n] += slope * std::log2(cached_spectrum_x_axis[n] / 1000.0);
        }

        const auto now = std::chrono::steady_clock::now();
        const double dt = last_spectrum_frame_time.has_value()
                              ? std::chrono::duration<double>(now - *last_spectrum_frame_time).count()
                              : 0.0;

        last_spectrum_frame_time = now;

        double range_min = min_level;
        double range_max = max_level;

        if (DbSpectrum::dynamicYScale()) {
          spectrum_auto_range.update(cached_spectrum_levels, dt, min_level, max_level,
                                     DbSpectrum::autoRangeMinimumSpan());

          range_min = spectrum_auto_range.low();
          range_max = spectrum_auto_range.high();
        } else {
          spectrum_auto_range.reset();
        }

        QList<QPointF> output_data(static_cast<qsizetype>(cached_spectrum_levels.size()));

        for (size_t n = 0; n < cached_spectrum_levels.size(); n++) {
          output_data[static_cast<qsizetype>(n)] =
              QPointF(cached_spectrum_x_axis[n], std::clamp(cached_spectrum_levels[n], range_min, range_max));
        }

        Q_EMIT newSpectrumData(output_data, range_min, range_max);
      },
      Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void EffectsBase::setUpdateLevelMeters(const bool& state) {
  output_level->updateLevelMeters = state;
}

void EffectsBase::setSpectrumBypass(const bool& state) {
  spectrum->bypass = state;
}
