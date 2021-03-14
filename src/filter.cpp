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

#include "filter.hpp"
#include <glibmm/main.h>
#include <array>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Filter* l) {
  const auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL = 0.0F;
            float inR = 0.0F;

            g_object_get(l->filter, "meter-inL", &inL, nullptr);
            g_object_get(l->filter, "meter-inR", &inR, nullptr);

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

            g_object_get(l->filter, "meter-outL", &outL, nullptr);
            g_object_get(l->filter, "meter-outR", &outR, nullptr);

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

Filter::Filter(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "filter", schema, schema_path) {
  filter = gst_element_factory_make("calf-sourceforge-net-plugins-Filter", "filter");

  if (is_installed(filter)) {
    auto* audioconvert_in = gst_element_factory_make("audioconvert", "filter_audioconvert_in");
    auto* audioconvert_out = gst_element_factory_make("audioconvert", "filter_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, filter, audioconvert_out, nullptr);

    gst_element_link_many(audioconvert_in, filter, audioconvert_out, nullptr);

    auto* pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto* pad_src = gst_element_get_static_pad(audioconvert_out, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(filter, "bypass", 0, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    if (g_settings_get_boolean(settings, "state") != 0) {
      enable();
    }
  }
}

Filter::~Filter() {
  util::debug(log_tag + name + " destroyed");
}

void Filter::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", filter, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", filter, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "frequency", filter, "freq", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "resonance", filter, "res", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "mode", filter, "mode", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "inertia", filter, "inertia", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);
}
