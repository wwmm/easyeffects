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

#include "reverb.hpp"
#include <glibmm/main.h>
#include <array>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Reverb* l) {
  const auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL = 0.0F;
            float inR = 0.0F;

            g_object_get(l->reverb, "meter-inL", &inL, nullptr);
            g_object_get(l->reverb, "meter-inR", &inR, nullptr);

            std::array<double, 2> in_peak = {inL, inR};

            l->input_level.emit(in_peak);

            return true;
          },
          100);
    }

    if (!l->output_level_connection.connected()) {
      l->output_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float outL = 0.0F;
            float outR = 0.0F;

            g_object_get(l->reverb, "meter-outL", &outL, nullptr);
            g_object_get(l->reverb, "meter-outR", &outR, nullptr);

            std::array<double, 2> out_peak = {outL, outR};

            l->output_level.emit(out_peak);

            return true;
          },
          100);
    }
  } else {
    l->input_level_connection.disconnect();
    l->output_level_connection.disconnect();
  }
}

}  // namespace

Reverb::Reverb(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "reverb", schema, schema_path) {
  reverb = gst_element_factory_make("calf-sourceforge-net-plugins-Reverb", "reverb");

  if (is_installed(reverb)) {
    auto* audioconvert_in = gst_element_factory_make("audioconvert", "reverb_audioconvert_in");
    auto* audioconvert_out = gst_element_factory_make("audioconvert", "reverb_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, reverb, audioconvert_out, nullptr);

    gst_element_link_many(audioconvert_in, reverb, audioconvert_out, nullptr);

    auto* pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto* pad_src = gst_element_get_static_pad(audioconvert_out, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(reverb, "on", 1, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    if (g_settings_get_boolean(settings, "state") != 0) {
      enable();
    }
  }
}

Reverb::~Reverb() {
  util::debug(log_tag + name + " destroyed");
}

void Reverb::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", reverb, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", reverb, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "room-size", reverb, "room-size", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "decay-time", reverb, "decay-time", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "hf-damp", reverb, "hf-damp", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "diffusion", reverb, "diffusion", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "amount", reverb, "amount", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "dry", reverb, "dry", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "predelay", reverb, "predelay", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "bass-cut", reverb, "bass-cut", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "treble-cut", reverb, "treble-cut", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);
}
