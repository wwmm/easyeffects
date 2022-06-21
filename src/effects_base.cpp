/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#include "effects_base.hpp"

EffectsBase::EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager)
    : log_tag(std::move(tag)),
      pm(pipe_manager),
      settings(g_settings_new(schema.c_str())),
      global_settings(g_settings_new(tags::app::id)) {
  std::string path = "/" + schema + "/";

  std::replace(path.begin(), path.end(), '.', '/');

  autogain = std::make_shared<AutoGain>(log_tag, tags::schema::autogain::id, path + "autogain/", pm);

  bass_enhancer = std::make_shared<BassEnhancer>(log_tag, tags::schema::bass_enhancer::id, path + "bassenhancer/", pm);

  bass_loudness = std::make_shared<BassLoudness>(log_tag, tags::schema::bass_loudness::id, path + "bassloudness/", pm);

  compressor = std::make_shared<Compressor>(log_tag, tags::schema::compressor::id, path + "compressor/", pm);

  convolver = std::make_shared<Convolver>(log_tag, tags::schema::convolver::id, path + "convolver/", pm);

  crossfeed = std::make_shared<Crossfeed>(log_tag, tags::schema::crossfeed::id, path + "crossfeed/", pm);

  crystalizer = std::make_shared<Crystalizer>(log_tag, tags::schema::crystalizer::id, path + "crystalizer/", pm);

  deesser = std::make_shared<Deesser>(log_tag, tags::schema::deesser::id, path + "deesser/", pm);

  delay = std::make_shared<Delay>(log_tag, tags::schema::delay::id, path + "delay/", pm);

  echo_canceller =
      std::make_shared<EchoCanceller>(log_tag, tags::schema::echo_canceller::id, path + "echocanceller/", pm);

  equalizer = std::make_shared<Equalizer>(log_tag, tags::schema::equalizer::id, path + "equalizer/",
                                          tags::schema::equalizer::channel_id, path + "equalizer/leftchannel/",
                                          path + "equalizer/rightchannel/", pm);

  exciter = std::make_shared<Exciter>(log_tag, tags::schema::exciter::id, path + "exciter/", pm);

  filter = std::make_shared<Filter>(log_tag, tags::schema::filter::id, path + "filter/", pm);

  gate = std::make_shared<Gate>(log_tag, tags::schema::gate::id, path + "gate/", pm);

  limiter = std::make_shared<Limiter>(log_tag, tags::schema::limiter::id, path + "limiter/", pm);

  loudness = std::make_shared<Loudness>(log_tag, tags::schema::loudness::id, path + "loudness/", pm);

  maximizer = std::make_shared<Maximizer>(log_tag, tags::schema::maximizer::id, path + "maximizer/", pm);

  multiband_compressor = std::make_shared<MultibandCompressor>(log_tag, tags::schema::multiband_compressor::id,
                                                               path + "multibandcompressor/", pm);

  multiband_gate =
      std::make_shared<MultibandGate>(log_tag, tags::schema::multiband_gate::id, path + "multibandgate/", pm);

  output_level = std::make_shared<OutputLevel>(log_tag, tags::schema::output_level::id, path + "outputlevel/", pm);

  pitch = std::make_shared<Pitch>(log_tag, tags::schema::pitch::id, path + "pitch/", pm);

  reverb = std::make_shared<Reverb>(log_tag, tags::schema::reverb::id, path + "reverb/", pm);

  rnnoise = std::make_shared<RNNoise>(log_tag, tags::schema::rnnoise::id, path + "rnnoise/", pm);

  spectrum = std::make_shared<Spectrum>(log_tag, tags::schema::spectrum::id, tags::app::path + "/spectrum/", pm);

  stereo_tools = std::make_shared<StereoTools>(log_tag, tags::schema::stereo_tools::id, path + "stereotools/", pm);

  if (!output_level->connected_to_pw) {
    output_level->connect_to_pw();
  }

  if (!spectrum->connected_to_pw) {
    spectrum->connect_to_pw();
  }

  plugins.insert(std::make_pair(autogain->name, autogain));
  plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer));
  plugins.insert(std::make_pair(bass_loudness->name, bass_loudness));
  plugins.insert(std::make_pair(compressor->name, compressor));
  plugins.insert(std::make_pair(convolver->name, convolver));
  plugins.insert(std::make_pair(crossfeed->name, crossfeed));
  plugins.insert(std::make_pair(crystalizer->name, crystalizer));
  plugins.insert(std::make_pair(deesser->name, deesser));
  plugins.insert(std::make_pair(delay->name, delay));
  plugins.insert(std::make_pair(echo_canceller->name, echo_canceller));
  plugins.insert(std::make_pair(equalizer->name, equalizer));
  plugins.insert(std::make_pair(exciter->name, exciter));
  plugins.insert(std::make_pair(filter->name, filter));
  plugins.insert(std::make_pair(gate->name, gate));
  plugins.insert(std::make_pair(limiter->name, limiter));
  plugins.insert(std::make_pair(loudness->name, loudness));
  plugins.insert(std::make_pair(maximizer->name, maximizer));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate));
  plugins.insert(std::make_pair(pitch->name, pitch));
  plugins.insert(std::make_pair(reverb->name, reverb));
  plugins.insert(std::make_pair(rnnoise->name, rnnoise));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools));

  connections.push_back(compressor->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(convolver->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(crystalizer->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(delay->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(echo_canceller->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(equalizer->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(limiter->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(loudness->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(maximizer->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(
      multiband_compressor->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(pitch->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  connections.push_back(rnnoise->latency.connect([=, this](const auto& v) { broadcast_pipeline_latency(); }));

  gconnections.push_back(g_signal_connect(settings, "changed::plugins",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<EffectsBase*>(user_data);

                                            self->broadcast_pipeline_latency();
                                          }),
                                          this));
}

EffectsBase::~EffectsBase() {
  for (auto& c : connections) {
    c.disconnect();
  }

  for (auto& handler_id : gconnections) {
    g_signal_handler_disconnect(settings, handler_id);
  }

  g_object_unref(settings);

  util::debug("effects_base: destroyed");
}

void EffectsBase::reset_settings() {
  util::reset_all_keys(settings);

  spectrum->reset_settings();

  for (auto& plugin : plugins | std::views::values) {
    plugin->reset_settings();
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
  float total = 0.0F;

  for (const auto& name : util::gchar_array_to_vector(g_settings_get_strv(settings, "plugins"))) {
    total += plugins[name]->get_latency_seconds();
  }

  return total * 1000.0F;
}

void EffectsBase::broadcast_pipeline_latency() {
  const auto latency_value = get_pipeline_latency();

  util::debug(log_tag + "pipeline latency: " + util::to_string(latency_value, "") + " ms");

  pipeline_latency.emit(latency_value);
}
