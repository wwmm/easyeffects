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

#include "exciter.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Exciter* l) {
  const auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->harmonics_connection.connected()) {
      l->harmonics_connection = Glib::signal_timeout().connect(
          [l]() {
            float harmonics = 0.0F;

            g_object_get(l->exciter, "meter-drive", &harmonics, nullptr);

            l->harmonics.emit(harmonics);

            return true;
          },
          100);
    }
  } else {
    l->harmonics_connection.disconnect();
  }
}

}  // namespace

Exciter::Exciter(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "exciter", schema, schema_path) {
  exciter = gst_element_factory_make("calf-sourceforge-net-plugins-Exciter", nullptr);

  if (is_installed(exciter)) {
    auto* in_level = gst_element_factory_make("level", "exciter_input_level");
    auto* out_level = gst_element_factory_make("level", "exciter_output_level");
    auto* audioconvert_in = gst_element_factory_make("audioconvert", "exciter_audioconvert_in");
    auto* audioconvert_out = gst_element_factory_make("audioconvert", "exciter_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), in_level, audioconvert_in, exciter, audioconvert_out, out_level, nullptr);
    gst_element_link_many(in_level, audioconvert_in, exciter, audioconvert_out, out_level, nullptr);

    auto* pad_sink = gst_element_get_static_pad(in_level, "sink");
    auto* pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(exciter, "bypass", 0, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Exciter::~Exciter() {
  util::debug(log_tag + name + " destroyed");
}

void Exciter::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", exciter, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", exciter, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "amount", exciter, "amount", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "harmonics", exciter, "drive", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "scope", exciter, "freq", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ceil", exciter, "ceil", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "blend", exciter, "blend", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind(settings, "ceil-active", exciter, "ceil-active", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "listen", exciter, "listen", G_SETTINGS_BIND_DEFAULT);
}
