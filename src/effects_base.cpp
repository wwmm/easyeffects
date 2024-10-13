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

#include "effects_base.hpp"
#include <qcontainerfwd.h>
#include <qtmetamacros.h>
#include <QPointer>
#include <QString>
#include <algorithm>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include "autogain.hpp"
#include "db_manager.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

// #include "bass_enhancer.hpp"
// #include "bass_loudness.hpp"
// #include "compressor.hpp"
// #include "convolver.hpp"
// #include "crossfeed.hpp"
// #include "crystalizer.hpp"
// #include "deepfilternet.hpp"
// #include "deesser.hpp"
// #include "delay.hpp"
// #include "echo_canceller.hpp"
// #include "equalizer.hpp"
// #include "exciter.hpp"
// #include "expander.hpp"
// #include "filter.hpp"
// #include "gate.hpp"
// #include "level_meter.hpp"
// #include "limiter.hpp"
// #include "loudness.hpp"
// #include "maximizer.hpp"
// #include "multiband_compressor.hpp"
// #include "multiband_gate.hpp"
// #include "output_level.hpp"
// #include "pitch.hpp"
// #include "reverb.hpp"
// #include "rnnoise.hpp"
// #include "spectrum.hpp"
// #include "speex.hpp"
// #include "stereo_tools.hpp"
// #include "tags_app.hpp"
// #include "tags_plugin_name.hpp"
// #include "tags_schema.hpp"

EffectsBase::EffectsBase(pw::Manager* pipe_manager, PipelineType pipe_type)
    : log_tag(pipe_type == PipelineType::output ? "soe: " : "sie: "), pm(pipe_manager), pipeline_type(pipe_type) {
  using namespace std::string_literals;

  // schema_base_path = "/" + schema + "/";

  // std::replace(schema_base_path.begin(), schema_base_path.end(), '.', '/');

  //   output_level = std::make_shared<OutputLevel>(log_tag, tags::schema::output_level::id,
  //                                                schema_base_path + "outputlevel/", pm, pipeline_type);

  //   spectrum = std::make_shared<Spectrum>(log_tag, tags::schema::spectrum::id, tags::app::path + "/spectrum/"s, pm,
  //                                         pipeline_type);

  //   if (!output_level->connected_to_pw) {
  //     output_level->connect_to_pw();
  //   }

  //   if (!spectrum->connected_to_pw) {
  //     spectrum->connect_to_pw();
  //   }

  create_filters_if_necessary();

  switch (pipeline_type) {
    case PipelineType::input:
      connect(db::StreamInputs::self(), &db::StreamInputs::pluginsChanged, [&]() {
        create_filters_if_necessary();
        broadcast_pipeline_latency();
      });
      break;
    case PipelineType::output:
      connect(db::StreamOutputs::self(), &db::StreamOutputs::pluginsChanged, [&]() {
        create_filters_if_necessary();
        broadcast_pipeline_latency();
      });
      break;
  }

  connect(db::Main::self(), &db::Main::metersUpdateIntervalChanged, [&]() {
    // spectrum->notification_time_window = 0.001F * db::Main::metersUpdateInterval();

    for (auto& plugin : plugins | std::views::values) {
      plugin->notification_time_window = 0.001F * db::Main::metersUpdateInterval();
    }
  });

  connect(db::Main::self(), &db::Main::lv2uiUpdateFrequencyChanged, [&]() {
    auto v = db::Main::lv2uiUpdateFrequency();

    for (auto& plugin : plugins | std::views::values) {
      plugin->set_native_ui_update_frequency(v);
    }
  });

  //   spectrum->notification_time_window = 0.001F *db::Main::metersUpdateInterval();

  for (auto& plugin : plugins | std::views::values) {
    if (!plugin.isNull()) {
      plugin->notification_time_window = 0.001F * db::Main::metersUpdateInterval();
    }
  }
}

EffectsBase::~EffectsBase() {
  util::debug("effects_base: destroyed");
}

void EffectsBase::reset_settings() {
  //   util::reset_all_keys_except(settings, {"input-device", "output-device"});

  //   spectrum->reset_settings();

  for (auto& plugin : plugins | std::views::values) {
    plugin->reset_settings();
  }
}

void EffectsBase::create_filters_if_necessary() {
  auto list = (pipeline_type == PipelineType::output ? db::StreamOutputs::plugins() : db::StreamInputs::plugins());

  if (list.empty()) {
    return;
  }

  for (const auto& name : list) {
    if (plugins.contains(name)) {
      continue;
    }

    auto instance_id = tags::plugin_name::get_id(name);

    QPointer<PluginBase> filter;

    if (name.startsWith(tags::plugin_name::BaseName::autogain)) {
      filter = new AutoGain(log_tag, pm, pipeline_type, instance_id);
    } else if (name.startsWith(tags::plugin_name::BaseName::bass_enhancer)) {
      //   filter = std::make_shared<BassEnhancer>(log_tag, tags::schema::bass_enhancer::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::bass_loudness)) {
      //   filter = std::make_shared<BassLoudness>(log_tag, tags::schema::bass_loudness::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::compressor)) {
      //   filter = std::make_shared<Compressor>(log_tag, tags::schema::compressor::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::convolver)) {
      //   filter = std::make_shared<Convolver>(log_tag, tags::schema::convolver::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::crossfeed)) {
      //   filter = std::make_shared<Crossfeed>(log_tag, tags::schema::crossfeed::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::crystalizer)) {
      //   filter = std::make_shared<Crystalizer>(log_tag, tags::schema::crystalizer::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::deepfilternet)) {
      //   filter = std::make_shared<DeepFilterNet>(log_tag, tags::schema::deepfilternet::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::deesser)) {
      //   filter = std::make_shared<Deesser>(log_tag, tags::schema::deesser::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::delay)) {
      //   filter = std::make_shared<Delay>(log_tag, tags::schema::delay::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::echo_canceller)) {
      //   filter = std::make_shared<EchoCanceller>(log_tag, tags::schema::echo_canceller::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::exciter)) {
      //   filter = std::make_shared<Exciter>(log_tag, tags::schema::exciter::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::expander)) {
      //   filter = std::make_shared<Expander>(log_tag, tags::schema::expander::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::equalizer)) {
      //   filter = std::make_shared<Equalizer>(
      //       log_tag, tags::schema::equalizer::id, path, tags::schema::equalizer::channel_id,
      //       schema_base_path + "equalizer/" + instance_id + "/leftchannel/",
      //       schema_base_path + "equalizer/" + instance_id + "/rightchannel/", pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::filter)) {
      //   filter = std::make_shared<Filter>(log_tag, tags::schema::filter::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::gate)) {
      //   filter = std::make_shared<Gate>(log_tag, tags::schema::gate::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::level_meter)) {
      //   filter = std::make_shared<LevelMeter>(log_tag, tags::schema::level_meter::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::limiter)) {
      //   filter = std::make_shared<Limiter>(log_tag, tags::schema::limiter::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::loudness)) {
      //   filter = std::make_shared<Loudness>(log_tag, tags::schema::loudness::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::maximizer)) {
      //   filter = std::make_shared<Maximizer>(log_tag, tags::schema::maximizer::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::multiband_compressor)) {
      //   filter = std::make_shared<MultibandCompressor>(log_tag, tags::schema::multiband_compressor::id, path, pm,
      //                                                  pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::multiband_gate)) {
      //   filter = std::make_shared<MultibandGate>(log_tag, tags::schema::multiband_gate::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::pitch)) {
      //   filter = std::make_shared<Pitch>(log_tag, tags::schema::pitch::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::reverb)) {
      //   filter = std::make_shared<Reverb>(log_tag, tags::schema::reverb::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::rnnoise)) {
      //   filter = std::make_shared<RNNoise>(log_tag, tags::schema::rnnoise::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::speex)) {
      //   filter = std::make_shared<Speex>(log_tag, tags::schema::speex::id, path, pm, pipeline_type);
    } else if (name.startsWith(tags::plugin_name::BaseName::stereo_tools)) {
      //   filter = std::make_shared<StereoTools>(log_tag, tags::schema::stereo_tools::id, path, pm, pipeline_type);
    }

    // connect(filter.get(), &PluginBase::latency, [this]() { broadcast_pipeline_latency(); });

    plugins.insert(std::make_pair(name, filter));
  }
}

void EffectsBase::remove_unused_filters() {
  auto list = (pipeline_type == PipelineType::output ? db::StreamOutputs::plugins() : db::StreamInputs::plugins());

  if (list.empty()) {
    plugins.clear();

    return;
  }

  for (auto it = plugins.begin(); it != plugins.end();) {
    auto key = it->first;

    if (std::ranges::find(list, key) == list.end()) {
      auto plugin = it->second;

      if (plugin.isNull()) {
        it = plugins.erase(it);

        continue;
      }

      plugin->bypass = true;
      plugin->set_post_messages(false);
      // plugin->latency.clear();

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

auto EffectsBase::get_pipeline_latency() -> float {
  auto list = (pipeline_type == PipelineType::output ? db::StreamOutputs::plugins() : db::StreamInputs::plugins());

  float total = 0.0F;

  for (const auto& name : list) {
    if (plugins.contains(name) && !plugins[name].isNull()) {
      total += plugins[name]->get_latency_seconds();
    }
  }

  return total * 1000.0F;
}

void EffectsBase::broadcast_pipeline_latency() {
  const auto latency_value = get_pipeline_latency();

  util::debug(log_tag + "pipeline latency: " + util::to_string(latency_value, "") + " ms");

  Q_EMIT pipeline_latency(latency_value);
}

auto EffectsBase::get_plugins_map() -> std::map<QString, QPointer<PluginBase>> {
  return plugins;
}