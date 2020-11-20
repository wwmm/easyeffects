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

#include "rnnoise.hpp"
#include <glibmm/main.h>
#include "util.hpp"

RNNoise::RNNoise(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "rnnoise", schema, schema_path) {
  rnnoise = gst_element_factory_make("pernnoise", nullptr);

  if (is_installed(rnnoise)) {
    auto* input_gain = gst_element_factory_make("volume", nullptr);
    auto* in_level = gst_element_factory_make("level", "rnnoise_input_level");
    auto* output_gain = gst_element_factory_make("volume", nullptr);
    auto* out_level = gst_element_factory_make("level", "rnnoise_output_level");
    capsfilter_out = gst_element_factory_make("capsfilter", nullptr);
    capsfilter_in = gst_element_factory_make("capsfilter", nullptr);
    auto* audioresample_in = gst_element_factory_make("audioresample", "rnnoise_audioresample_in");
    auto* audioresample_out = gst_element_factory_make("audioresample", "rnnoise_audioresample_out");
    auto* adapter = gst_element_factory_make("peadapter", nullptr);

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioresample_in, capsfilter_in, adapter, rnnoise,
                     audioresample_out, capsfilter_out, output_gain, out_level, nullptr);

    gst_element_link_many(input_gain, in_level, audioresample_in, capsfilter_in, adapter, rnnoise, audioresample_out,
                          capsfilter_out, output_gain, out_level, nullptr);

    auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto* pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(adapter, "blocksize", 480, nullptr);

    set_caps_in();

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
}

RNNoise::~RNNoise() {
  util::debug(log_tag + name + " destroyed");
}

void RNNoise::bind_to_gsettings() {
  g_settings_bind(settings, "model-path", rnnoise, "model-path", G_SETTINGS_BIND_DEFAULT);
}

void RNNoise::set_caps_out(const uint& sampling_rate) {
  auto caps_str = "audio/x-raw,format=F32LE,channels=2,rate=" + std::to_string(sampling_rate);

  auto* caps = gst_caps_from_string(caps_str.c_str());

  g_object_set(capsfilter_out, "caps", caps, nullptr);

  gst_caps_unref(caps);
}

void RNNoise::set_caps_in() {
  auto* caps = gst_caps_from_string("audio/x-raw,format=F32LE,channels=2,rate=48000");

  g_object_set(capsfilter_in, "caps", caps, nullptr);

  gst_caps_unref(caps);
}