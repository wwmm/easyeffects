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

#include "convolver.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_n_input_samples_changed(GObject* gobject, GParamSpec* pspec, Convolver* c) {
  int v = 0;
  int blocksize = 0;

  g_object_get(c->adapter, "n-input-samples", &v, nullptr);
  g_object_get(c->adapter, "blocksize", &blocksize, nullptr);

  util::debug(c->log_tag + "convolver: new input block size " + std::to_string(v) + " frames");
}

}  // namespace

Convolver::Convolver(const std::string& tag, const std::string& schema, const std::string& schema_path)
    : PluginBase(tag, "convolver", schema, schema_path) {
  convolver = gst_element_factory_make("peconvolver", "convolver");

  if (is_installed(convolver)) {
    auto* input_gain = gst_element_factory_make("volume", nullptr);
    auto* in_level = gst_element_factory_make("level", "convolver_input_level");
    auto* out_level = gst_element_factory_make("level", "convolver_output_level");
    auto* output_gain = gst_element_factory_make("volume", nullptr);
    auto* audioconvert_in = gst_element_factory_make("audioconvert", "convolver_audioconvert_in");
    auto* audioconvert_out = gst_element_factory_make("audioconvert", "convolver_audioconvert_out");
    adapter = gst_element_factory_make("peadapter", nullptr);

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, adapter, audioconvert_in, convolver, audioconvert_out,
                     output_gain, out_level, nullptr);

    gst_element_link_many(input_gain, in_level, adapter, audioconvert_in, convolver, audioconvert_out, output_gain,
                          out_level, nullptr);

    auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto* pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(adapter, "blocksize", 512, nullptr);
    g_object_set(adapter, "passthrough", 1, nullptr);

    g_signal_connect(adapter, "notify::n-input-samples", G_CALLBACK(on_n_input_samples_changed), this);

    bind_to_gsettings();

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

Convolver::~Convolver() {
  util::debug(log_tag + name + " destroyed");
}

void Convolver::bind_to_gsettings() {
  g_settings_bind(settings, "kernel-path", convolver, "kernel-path", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "ir-width", convolver, "ir-width", G_SETTINGS_BIND_DEFAULT);
}
