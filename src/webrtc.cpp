/*
 *  Copyright © 2017-2020 Wellington Wallace
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

#include "webrtc.hpp"

// namespace {

// void on_n_input_samples_changed(GObject* gobject, GParamSpec* pspec, Webrtc* w) {
//   int v = 0;
//   int blocksize = 0;

//   g_object_get(w->adapter, "n-input-samples", &v, nullptr);
//   g_object_get(w->adapter, "blocksize", &blocksize, nullptr);

//   util::debug(w->log_tag + "webrtc: new input block size " + std::to_string(v) + " frames");
//   util::debug(w->log_tag + "webrtc: we will try to read in chunks of " + std::to_string(blocksize) + " frames");

//   g_object_set(w->adapter_out, "blocksize", v, nullptr);
// }

// }  // namespace

Webrtc::Webrtc(const std::string& tag,
               const std::string& schema,
               const std::string& schema_path,
               const int& sampling_rate,
               PipeManager* pipe_manager)
    : PluginBase(tag, "webrtc", schema, schema_path, pipe_manager), rate(sampling_rate) {
  // webrtc = gst_element_factory_make("webrtcdsp", nullptr);

  // if (is_installed(webrtc)) {
  //   build_probe_bin();
  //   build_dsp_bin();

  //   bind_to_gsettings();

  //   // useless write just to force callback call

  //   auto enable = g_settings_get_boolean(settings, "state");

  //   g_settings_set_boolean(settings, "state", enable);
  // }
}

Webrtc::~Webrtc() {
  util::debug(log_tag + name + " destroyed");
}

void Webrtc::build_probe_bin() {
  // probe_bin = gst_bin_new("probe_bin");

  // probe_src = gst_element_factory_make("pipewiresrc", nullptr);
  // auto* queue = gst_element_factory_make("queue", nullptr);
  // auto* audioconvert = gst_element_factory_make("audioconvert", nullptr);
  // auto* audioresample = gst_element_factory_make("audioresample", nullptr);
  // auto* capsfilter = gst_element_factory_make("capsfilter", nullptr);
  // auto* probe = gst_element_factory_make("webrtcechoprobe", nullptr);
  // auto* sink = gst_element_factory_make("fakesink", nullptr);

  // auto* props = gst_structure_from_string("props,application.name=EasyEffectsWebrtcProbe", nullptr);

  // const auto* caps_str = "audio/x-raw,format=S16LE,channels=2,rate=48000";
  // auto* caps = gst_caps_from_string(caps_str);

  // g_object_set(probe_src, "always-copy", 1, nullptr);
  // g_object_set(probe_src, "do-timestamp", 1, nullptr);
  // g_object_set(probe_src, "stream-properties", props, nullptr);

  // g_object_set(capsfilter, "caps", caps, nullptr);

  // g_object_set(queue, "silent", 1, nullptr);

  // gst_structure_free(props);
  // gst_caps_unref(caps);

  // gst_bin_add_many(GST_BIN(probe_bin), probe_src, queue, audioconvert, audioresample, capsfilter, probe, sink,
  // nullptr);

  // gst_element_link_many(probe_src, queue, audioconvert, audioresample, capsfilter, probe, sink, nullptr);
}

void Webrtc::build_dsp_bin() {
  // auto* in_level = gst_element_factory_make("level", "webrtc_input_level");
  // auto* audioconvert_in = gst_element_factory_make("audioconvert", nullptr);
  // auto* audioresample_in = gst_element_factory_make("audioresample", nullptr);
  // auto* caps_in = gst_element_factory_make("capsfilter", nullptr);
  // auto* audioconvert_out = gst_element_factory_make("audioconvert", nullptr);
  // auto* audioresample_out = gst_element_factory_make("audioresample", nullptr);
  // auto* caps_out = gst_element_factory_make("capsfilter", nullptr);
  // auto* out_level = gst_element_factory_make("level", "webrtc_output_level");

  // adapter = gst_element_factory_make("peadapter", nullptr);
  // adapter_out = gst_element_factory_make("peadapter", nullptr);

  // auto* capsin = gst_caps_from_string("audio/x-raw,channels=2,format=S16LE,rate=48000");
  // auto* capsout = gst_caps_from_string(("audio/x-raw,channels=2,format=F32LE,rate=" + std::to_string(rate)).c_str());

  // g_object_set(caps_in, "caps", capsin, nullptr);
  // g_object_set(caps_out, "caps", capsout, nullptr);

  // gst_caps_unref(capsin);
  // gst_caps_unref(capsout);

  // g_object_set(adapter, "blocksize", 480, nullptr);

  // gst_bin_add(GST_BIN(bin), probe_bin);

  // gst_bin_add_many(GST_BIN(bin), in_level, adapter, audioconvert_in, audioresample_in, caps_in, webrtc, adapter_out,
  //                  audioconvert_out, audioresample_out, caps_out, out_level, nullptr);

  // gst_element_link_many(in_level, adapter, audioconvert_in, audioresample_in, caps_in, webrtc, audioconvert_out,
  //                       audioresample_out, caps_out, adapter_out, out_level, nullptr);

  // auto* pad_sink = gst_element_get_static_pad(in_level, "sink");
  // auto* pad_src = gst_element_get_static_pad(out_level, "src");

  // gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
  // gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

  // gst_object_unref(GST_OBJECT(pad_sink));
  // gst_object_unref(GST_OBJECT(pad_src));

  // g_signal_connect(adapter, "notify::n-input-samples", G_CALLBACK(on_n_input_samples_changed), this);

  // g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
  // g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
}

void Webrtc::set_probe_input_node_id(const uint& id) const {
  // if (probe_src != nullptr) {
  //   auto path = std::to_string(id);

  //   g_object_set(probe_src, "path", path.c_str(), nullptr);

  //   util::debug(log_tag + "webrtc probe is using input device: " + path);
  // }
}

void Webrtc::bind_to_gsettings() {
  // g_settings_bind(settings, "high-pass-filter", webrtc, "high-pass-filter", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "echo-cancel", webrtc, "echo-cancel", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "echo-suppression-level", webrtc, "echo-suppression-level", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "noise-suppression", webrtc, "noise-suppression", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "noise-suppression-level", webrtc, "noise-suppression-level", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "gain-control", webrtc, "gain-control", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "extended-filter", webrtc, "extended-filter", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "delay-agnostic", webrtc, "delay-agnostic", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "target-level-dbfs", webrtc, "target-level-dbfs", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "compression-gain-db", webrtc, "compression-gain-db", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "limiter", webrtc, "limiter", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "voice-detection", webrtc, "voice-detection", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "gain-control-mode", webrtc, "gain-control-mode", G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "voice-detection-frame-size-ms", webrtc, "voice-detection-frame-size-ms",
  //                 G_SETTINGS_BIND_DEFAULT);

  // g_settings_bind(settings, "voice-detection-likelihood", webrtc, "voice-detection-likelihood",
  //                 G_SETTINGS_BIND_DEFAULT);
}
