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

#include "crystalizer.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Crystalizer* l) {
  const auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->range_before_connection.connected()) {
      l->range_before_connection = Glib::signal_timeout().connect(
          [l]() {
            float v = 0.0F;

            g_object_get(l->crystalizer, "lra-before", &v, nullptr);

            l->range_before.emit(v);

            return true;
          },
          100);
    }

    if (!l->range_after_connection.connected()) {
      l->range_after_connection = Glib::signal_timeout().connect(
          [l]() {
            float v = 0.0F;

            g_object_get(l->crystalizer, "lra-after", &v, nullptr);

            l->range_after.emit(v);

            return true;
          },
          100);
    }
  } else {
    l->range_before_connection.disconnect();
    l->range_after_connection.disconnect();
  }
}

}  // namespace

Crystalizer::Crystalizer(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "crystalizer", schema, schema_path) {
  crystalizer = gst_element_factory_make("pecrystalizer", nullptr);

  if (is_installed(crystalizer)) {
    auto* input_gain = gst_element_factory_make("volume", nullptr);
    auto* in_level = gst_element_factory_make("level", "crystalizer_input_level");
    auto* output_gain = gst_element_factory_make("volume", nullptr);
    auto* out_level = gst_element_factory_make("level", "crystalizer_output_level");

    auto* audioconvert_in = gst_element_factory_make("audioconvert", "crystalizer_audioconvert_in");
    auto* audioconvert_out = gst_element_factory_make("audioconvert", "crystalizer_audioconvert_out");

    adapter = gst_element_factory_make("peadapter", nullptr);

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, adapter, audioconvert_in, crystalizer, audioconvert_out,
                     output_gain, out_level, nullptr);

    gst_element_link_many(input_gain, in_level, adapter, audioconvert_in, crystalizer, audioconvert_out, output_gain,
                          out_level, nullptr);

    auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto* pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(adapter, "blocksize", 512, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    if (g_settings_get_boolean(settings, "state") != 0) {
      enable();
    }
  }
}

Crystalizer::~Crystalizer() {
  util::debug(log_tag + name + " destroyed");
}

void Crystalizer::bind_to_gsettings() {
  g_settings_bind(settings, "post-messages", crystalizer, "notify-host", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "aggressive", crystalizer, "aggressive", G_SETTINGS_BIND_DEFAULT);

  for (int n = 0; n < 13; n++) {
    g_settings_bind_with_mapping(settings, std::string("intensity-band" + std::to_string(n)).c_str(), crystalizer,
                                 std::string("intensity-band" + std::to_string(n)).c_str(), G_SETTINGS_BIND_GET,
                                 util::db20_gain_to_linear, nullptr, nullptr, nullptr);

    g_settings_bind(settings, std::string("mute-band" + std::to_string(n)).c_str(), crystalizer,
                    std::string("mute-band" + std::to_string(n)).c_str(), G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, std::string("bypass-band" + std::to_string(n)).c_str(), crystalizer,
                    std::string("bypass-band" + std::to_string(n)).c_str(), G_SETTINGS_BIND_DEFAULT);
  }
}
