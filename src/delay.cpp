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

#include "delay.hpp"

namespace {

void on_process(void* userdata, struct spa_io_position* position) {
  // auto* d = static_cast<data*>(userdata);

  // uint32_t n_samples = position->clock.duration;

  // pw_log_trace("do process %d", n_samples);
  util::warning("processing");

  // auto* in_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_left, n_samples));
  // auto* in_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_right, n_samples));

  // auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  // auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  // memcpy(out_left, in_left, n_samples * sizeof(float));
  // memcpy(out_right, in_right, n_samples * sizeof(float));
}

// void destroy_filter(void* data) {
// auto* pf = static_cast<PipeFilter*>(data);

// util::debug(pf->log_tag + "Destroying Pipewire filter...");

// spa_hook_remove(&pf->listener);
// }

static const struct pw_filter_events filter_events = {
    PW_VERSION_FILTER_EVENTS,
    .process = on_process,
};

}  // namespace

Delay::Delay(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager)
    : PluginBase(tag, "delay", schema, schema_path), pm(pipe_manager) {
  delay = gst_element_factory_make("lsp-plug-in-plugins-lv2-comp-delay-x2-stereo", nullptr);

  if (is_installed(delay)) {
    auto* input_gain = gst_element_factory_make("volume", nullptr);
    auto* in_level = gst_element_factory_make("level", "delay_input_level");
    auto* output_gain = gst_element_factory_make("volume", nullptr);
    auto* out_level = gst_element_factory_make("level", "delay_output_level");
    auto* audioconvert_in = gst_element_factory_make("audioconvert", "delay_audioconvert_in");
    auto* audioconvert_out = gst_element_factory_make("audioconvert", "delay_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, delay, audioconvert_out, output_gain,
                     out_level, nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, delay, audioconvert_out, output_gain, out_level,
                          nullptr);

    auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto* pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(delay, "enabled", 1, nullptr);
    g_object_set(delay, "mode-l", 2, nullptr);
    g_object_set(delay, "mode-r", 2, nullptr);
    g_object_set(delay, "dry-l", 0.0F, nullptr);
    g_object_set(delay, "dry-r", 0.0F, nullptr);
    g_object_set(delay, "wet-l", 1.0F, nullptr);
    g_object_set(delay, "wet-r", 1.0F, nullptr);
    g_object_set(delay, "g-out", 1.0F, nullptr);

    bind_to_gsettings();

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }

  // testing new ideas

  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_NODE_NAME, "pe_filter_delay");
  pw_properties_set(props_filter, PW_KEY_NODE_NICK, tag.c_str());
  pw_properties_set(props_filter, PW_KEY_NODE_DESCRIPTION, "pulseeffects_filter");
  pw_properties_set(props_filter, PW_KEY_NODE_TARGET, std::to_string(pm->pe_sink_node.id).c_str());
  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");

  filter = pw_filter_new(pm->core, "pe_filter_delay", props_filter);

  pw_filter_add_listener(filter, &listener, &filter_events, &pf_data);

  // left channel input

  auto* props_in_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_left, PW_KEY_PORT_NAME, "input_fl");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  pf_data.in_left = static_cast<pf::port*>(pw_filter_add_port(
      filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(pf::port), props_in_left, nullptr, 0));

  // right channel input

  auto* props_in_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_right, PW_KEY_PORT_NAME, "input_fr");
  pw_properties_set(props_in_right, "audio.channel", "FR");

  pf_data.in_right = static_cast<pf::port*>(pw_filter_add_port(
      filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(pf::port), props_in_right, nullptr, 0));

  // left channel output

  auto* props_out_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_left, PW_KEY_PORT_NAME, "output_fl");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  pf_data.out_left = static_cast<pf::port*>(pw_filter_add_port(
      filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(pf::port), props_out_left, nullptr, 0));

  // right channel output

  auto* props_out_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_right, PW_KEY_PORT_NAME, "output_fr");
  pw_properties_set(props_in_left, "audio.channel", "FR");

  pf_data.out_right = static_cast<pf::port*>(pw_filter_add_port(
      filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS, sizeof(pf::port), props_out_right, nullptr, 0));

  pw_thread_loop_lock(pm->thread_loop);

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0) {
    util::error(log_tag + "can not connect the filter to pipewire!");
  } else {
    util::debug(log_tag + "delay filter connected to pipewire");
  }

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

Delay::~Delay() {
  util::debug(log_tag + name + " destroyed");
}

void Delay::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "time-l", delay, "time-l", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "time-r", delay, "time-r", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);
}
