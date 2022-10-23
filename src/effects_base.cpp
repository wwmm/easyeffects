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

#include "effects_base.hpp"

EffectsBase::EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager)
    : log_tag(std::move(tag)),
      pm(pipe_manager),
      settings(g_settings_new(schema.c_str())),
      global_settings(g_settings_new(tags::app::id)) {
  using namespace std::string_literals;

  schema_base_path = "/" + schema + "/";

  std::replace(schema_base_path.begin(), schema_base_path.end(), '.', '/');

  output_level =
      std::make_shared<OutputLevel>(log_tag, tags::schema::output_level::id, schema_base_path + "outputlevel/", pm);

  spectrum = std::make_shared<Spectrum>(log_tag, tags::schema::spectrum::id, tags::app::path + "/spectrum/"s, pm);

  if (!output_level->connected_to_pw) {
    output_level->connect_to_pw();
  }

  if (!spectrum->connected_to_pw) {
    spectrum->connect_to_pw();
  }

  create_filters_if_necessary();

  gconnections.push_back(g_signal_connect(settings, "changed::plugins",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<EffectsBase*>(user_data);

                                            self->create_filters_if_necessary();

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
  util::reset_all_keys_except(settings, {"input-device", "output-device"});

  spectrum->reset_settings();

  for (auto& plugin : plugins | std::views::values) {
    plugin->reset_settings();
  }
}

void EffectsBase::create_filters_if_necessary() {
  const auto list = util::gchar_array_to_vector(g_settings_get_strv(settings, "plugins"));

  if (list.empty()) {
    return;
  }

  for (const auto& name : list) {
    if (plugins.contains(name)) {
      continue;
    }

    auto instance_id = util::to_string(tags::plugin_name::get_id(name));

    auto path = schema_base_path + tags::plugin_name::get_base_name(name) + "/" + instance_id + "/";

    path.erase(std::remove(path.begin(), path.end(), '_'), path.end());

    std::shared_ptr<PluginBase> filter;

    if (name.starts_with(tags::plugin_name::autogain)) {
      filter = std::make_shared<AutoGain>(log_tag, tags::schema::autogain::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::bass_enhancer)) {
      filter = std::make_shared<BassEnhancer>(log_tag, tags::schema::bass_enhancer::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::bass_loudness)) {
      filter = std::make_shared<BassLoudness>(log_tag, tags::schema::bass_loudness::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::compressor)) {
      filter = std::make_shared<Compressor>(log_tag, tags::schema::compressor::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::convolver)) {
      filter = std::make_shared<Convolver>(log_tag, tags::schema::convolver::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::crossfeed)) {
      filter = std::make_shared<Crossfeed>(log_tag, tags::schema::crossfeed::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::crystalizer)) {
      filter = std::make_shared<Crystalizer>(log_tag, tags::schema::crystalizer::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::deesser)) {
      filter = std::make_shared<Deesser>(log_tag, tags::schema::deesser::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::delay)) {
      filter = std::make_shared<Delay>(log_tag, tags::schema::delay::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::echo_canceller)) {
      filter = std::make_shared<EchoCanceller>(log_tag, tags::schema::echo_canceller::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::exciter)) {
      filter = std::make_shared<Exciter>(log_tag, tags::schema::exciter::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::equalizer)) {
      filter =
          std::make_shared<Equalizer>(log_tag, tags::schema::equalizer::id, path, tags::schema::equalizer::channel_id,
                                      schema_base_path + "equalizer/" + instance_id + "/leftchannel/",
                                      schema_base_path + "equalizer/" + instance_id + "/rightchannel/", pm);
    } else if (name.starts_with(tags::plugin_name::filter)) {
      filter = std::make_shared<Filter>(log_tag, tags::schema::filter::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::gate)) {
      filter = std::make_shared<Gate>(log_tag, tags::schema::gate::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::limiter)) {
      filter = std::make_shared<Limiter>(log_tag, tags::schema::limiter::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::loudness)) {
      filter = std::make_shared<Loudness>(log_tag, tags::schema::loudness::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::maximizer)) {
      filter = std::make_shared<Maximizer>(log_tag, tags::schema::maximizer::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::multiband_compressor)) {
      filter = std::make_shared<MultibandCompressor>(log_tag, tags::schema::multiband_compressor::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::multiband_gate)) {
      filter = std::make_shared<MultibandGate>(log_tag, tags::schema::multiband_gate::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::pitch)) {
      filter = std::make_shared<Pitch>(log_tag, tags::schema::pitch::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::reverb)) {
      filter = std::make_shared<Reverb>(log_tag, tags::schema::reverb::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::rnnoise)) {
      filter = std::make_shared<RNNoise>(log_tag, tags::schema::rnnoise::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::speex)) {
      filter = std::make_shared<Speex>(log_tag, tags::schema::speex::id, path, pm);
    } else if (name.starts_with(tags::plugin_name::stereo_tools)) {
      filter = std::make_shared<StereoTools>(log_tag, tags::schema::stereo_tools::id, path, pm);
    }

    connections.push_back(filter->latency.connect([=, this]() { broadcast_pipeline_latency(); }));

    plugins.insert(std::make_pair(name, filter));
  }
}

void EffectsBase::remove_unused_filters() {
  const auto list = util::gchar_array_to_vector(g_settings_get_strv(settings, "plugins"));

  if (list.empty()) {
    plugins.clear();

    return;
  }

  for (auto it = plugins.begin(); it != plugins.end();) {
    auto key = it->first;

    if (std::ranges::find(list, key) == list.end()) {
      auto plugin = it->second;

      plugin->bypass = true;
      plugin->set_post_messages(false);
      plugin->latency.clear();

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
  float total = 0.0F;

  for (const auto& name : util::gchar_array_to_vector(g_settings_get_strv(settings, "plugins"))) {
    if (plugins.contains(name)) {
      total += plugins[name]->get_latency_seconds();
    }
  }

  return total * 1000.0F;
}

void EffectsBase::broadcast_pipeline_latency() {
  const auto latency_value = get_pipeline_latency();

  util::debug(log_tag + "pipeline latency: " + util::to_string(latency_value, "") + " ms");

  pipeline_latency.emit(latency_value);
}
