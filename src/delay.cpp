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

Delay::Delay(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "delay", schema, schema_path) {
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
