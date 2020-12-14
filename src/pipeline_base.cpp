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

#include "pipeline_base.hpp"
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <sys/resource.h>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <string>
#include "config.h"
#include "gst/gstelement.h"
#include "gst/gstmessage.h"
#include "util.hpp"

namespace {

void on_message_error(const GstBus* gst_bus, GstMessage* message, PipelineBase* pb) {
  GError* err = nullptr;
  gchar* debug = nullptr;

  gst_message_parse_error(message, &err, &debug);

  util::critical(pb->log_tag + err->message);
  util::debug(pb->log_tag + debug);

  pb->set_null_pipeline();

  g_error_free(err);
  g_free(debug);
}

void on_stream_status(GstBus* bus, GstMessage* message, PipelineBase* pb) {
  GstStreamStatusType type = GST_STREAM_STATUS_TYPE_DESTROY;
  GstElement* owner = nullptr;

  gchar* path = nullptr;
  std::string path_str;
  std::string source_name;
  std::size_t idx = 0;

  int priority = 4;
  int niceness = -10;
  int priority_type = 2;  // None

  gst_message_parse_stream_status(message, &type, &owner);

  switch (type) {
    case GST_STREAM_STATUS_TYPE_ENTER: {
      path = gst_object_get_path_string(GST_OBJECT(owner));

      path_str = path;

      idx = path_str.find_last_of('/');

      source_name = path_str.substr(idx + 1);

      g_free(path);

      priority_type = g_settings_get_enum(pb->settings, "priority-type");

      switch (priority_type) {
        case 0: {  // Niceness (high priority in rtkit terms)
          niceness = g_settings_get_int(pb->settings, "niceness");

          pb->rtkit->set_nice(source_name, niceness);

          break;
        }
        case 1: {  // Real Time
          priority = g_settings_get_int(pb->settings, "realtime-priority");

          pb->rtkit->set_priority(source_name, priority);
        }
      }
    }
    default:
      break;
  }
}

void on_message_state_changed(const GstBus* gst_bus, GstMessage* message, PipelineBase* pb) {
  if (std::strcmp(GST_OBJECT_NAME(message->src), "pipeline") == 0) {
    GstState old_state = GST_STATE_NULL;
    GstState new_state = GST_STATE_NULL;
    GstState pending = GST_STATE_NULL;

    gst_message_parse_state_changed(message, &old_state, &new_state, &pending);

    util::debug(pb->log_tag + gst_element_state_get_name(old_state) + " -> " + gst_element_state_get_name(new_state) +
                " -> " + gst_element_state_get_name(pending));

    if (new_state == GST_STATE_PLAYING) {
      pb->playing = true;

      pb->get_latency();
    } else {
      pb->playing = false;
    }
  }
}

void on_message_latency(const GstBus* gst_bus, GstMessage* message, PipelineBase* pb) {
  if (std::strcmp(GST_OBJECT_NAME(message->src), "source") == 0) {
    int latency = 0;
    int buffer = 0;

    g_object_get(pb->source, "latency-time", &latency, nullptr);
    g_object_get(pb->source, "buffer-time", &buffer, nullptr);

    util::debug(pb->log_tag + "pulsesrc latency [us]: " + std::to_string(latency));
    util::debug(pb->log_tag + "pulsesrc buffer [us]: " + std::to_string(buffer));
  } else if (std::strcmp(GST_OBJECT_NAME(message->src), "sink") == 0) {
    int latency = 0;
    int buffer = 0;

    g_object_get(pb->sink, "latency-time", &latency, nullptr);
    g_object_get(pb->sink, "buffer-time", &buffer, nullptr);

    util::debug(pb->log_tag + "pulsesink latency [us]: " + std::to_string(latency));
    util::debug(pb->log_tag + "pulsesink buffer [us]: " + std::to_string(buffer));
  }

  pb->get_latency();
}

void on_message_element(const GstBus* gst_bus, GstMessage* message, PipelineBase* pb) {
  if (std::strcmp(GST_OBJECT_NAME(message->src), "spectrum") == 0 && !pb->resizing_spectrum) {
    const GstStructure* s = gst_message_get_structure(message);

    const GValue* magnitudes = nullptr;

    magnitudes = gst_structure_get_value(s, "magnitude");

    for (uint n = 0U; n < pb->spectrum_freqs.size(); n++) {
      pb->spectrum_mag_tmp[n] = g_value_get_float(gst_value_list_get_value(magnitudes, n + pb->spectrum_start_index));
    }

    try {
      boost::math::interpolators::cardinal_cubic_b_spline<float> spline(
          pb->spectrum_mag_tmp.begin(), pb->spectrum_mag_tmp.end(), pb->spline_f0, pb->spline_df);

      for (uint n = 0U; n < pb->spectrum_mag.size(); n++) {
        pb->spectrum_mag[n] = spline(pb->spectrum_x_axis[n]);
      }
    } catch (const std::exception& e) {
      util::debug(std::string("Message from thrown exception was: ") + e.what());
    }

    auto min_mag = pb->spectrum_threshold;
    auto max_mag = *std::max_element(pb->spectrum_mag.begin(), pb->spectrum_mag.end());

    if (max_mag > min_mag) {
      for (float& v : pb->spectrum_mag) {
        if (min_mag < v) {
          v = (min_mag - v) / min_mag;
        } else {
          v = 0.0F;
        }
      }

      Glib::signal_idle().connect_once([=] { pb->new_spectrum.emit(pb->spectrum_mag); });
    }
  }
}

void on_spectrum_n_points_changed(GSettings* settings, gchar* key, PipelineBase* pb) {
  unsigned long int npoints = g_settings_get_int(settings, "n-points");

  if (npoints != pb->spectrum_mag.size()) {
    pb->resizing_spectrum = true;

    pb->spectrum_mag.resize(npoints);

    pb->spectrum_x_axis = util::logspace(log10(static_cast<float>(pb->min_spectrum_freq)),
                                         log10(static_cast<float>(pb->max_spectrum_freq)), npoints);

    pb->resizing_spectrum = false;
  }
}

void on_src_type_changed(GstElement* typefind, guint probability, GstCaps* caps, PipelineBase* pb) {
  GstStructure* structure = gst_caps_get_structure(caps, 0);

  int rate = 44100;

  gst_structure_get_int(structure, "rate", &rate);

  pb->sampling_rate = rate;

  pb->init_spectrum();

  util::debug(pb->log_tag + "sampling rate: " + std::to_string(rate) + " Hz");
}

void on_buffer_changed(GObject* gobject, GParamSpec* pspec, PipelineBase* pb) {
  GstState state = GST_STATE_NULL;
  GstState pending = GST_STATE_NULL;

  gst_element_get_state(pb->pipeline, &state, &pending, pb->state_check_timeout);

  if (state == GST_STATE_PLAYING || state == GST_STATE_PAUSED) {
    /* when we are playing it is necessary to reset the pipeline for the new
     * value to take effect
     */

    gst_element_set_state(pb->pipeline, GST_STATE_READY);

    pb->update_pipeline_state();
  }
}

void on_latency_changed(GObject* gobject, GParamSpec* pspec, PipelineBase* pb) {
  GstState state = GST_STATE_NULL;
  GstState pending = GST_STATE_NULL;

  gst_element_get_state(pb->pipeline, &state, &pending, pb->state_check_timeout);

  if (state == GST_STATE_PLAYING || state == GST_STATE_PAUSED) {
    /* when we are playing it is necessary to reset the pipeline for the new
     * value to take effect
     */

    gst_element_set_state(pb->pipeline, GST_STATE_READY);

    pb->update_pipeline_state();
  }
}

auto on_sink_event(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
  GstEvent* event = GST_PAD_PROBE_INFO_EVENT(info);

  if (event->type == GST_EVENT_LATENCY) {
    auto* pb = static_cast<PipelineBase*>(user_data);

    pb->get_latency();
  }

  return GST_PAD_PROBE_PASS;
}

void on_bypass_on(gpointer user_data) {
  auto* pb = static_cast<PipelineBase*>(user_data);

  auto* effects_bin = gst_bin_get_by_name(GST_BIN(pb->pipeline), "effects_bin");

  if (effects_bin != nullptr) {
    gst_element_set_state(effects_bin, GST_STATE_READY);

    gst_element_unlink_many(pb->src_type, effects_bin, pb->spectrum_bin, pb->global_level_meter_bin, nullptr);

    gst_bin_remove(GST_BIN(pb->pipeline), effects_bin);

    gst_element_link_many(pb->src_type, pb->spectrum_bin, pb->global_level_meter_bin, nullptr);

    util::debug(pb->log_tag + " bypass enabled");
  } else {
    util::debug(pb->log_tag + "bypass is already enabled");
  }
}

void on_bypass_off(gpointer user_data) {
  auto* pb = static_cast<PipelineBase*>(user_data);

  auto* bin = gst_bin_get_by_name(GST_BIN(pb->pipeline), "effects_bin");

  if (bin == nullptr) {
    gst_element_set_state(pb->effects_bin, GST_STATE_NULL);

    gst_element_unlink_many(pb->src_type, pb->spectrum_bin, pb->global_level_meter_bin, nullptr);

    gst_bin_add(GST_BIN(pb->pipeline), pb->effects_bin);

    gst_element_link_many(pb->src_type, pb->effects_bin, pb->spectrum_bin, pb->global_level_meter_bin, nullptr);

    gst_element_sync_state_with_parent(pb->effects_bin);

    util::debug(pb->log_tag + " bypass disabled");
  } else {
    util::debug(pb->log_tag + "bypass is already disabled");
  }
}

auto bypass_event_probe_cb(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
  if (GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_CUSTOM_DOWNSTREAM) {
    return GST_PAD_PROBE_PASS;
  }

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  on_bypass_on(user_data);

  return GST_PAD_PROBE_DROP;
}

auto bypass_on_pad_blocked(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
  auto* pb = static_cast<PipelineBase*>(user_data);

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  auto* srcpad = gst_element_get_static_pad(pb->spectrum_bin, "src");

  gst_pad_add_probe(srcpad,
                    static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
                    bypass_event_probe_cb, user_data, nullptr);

  auto* sinkpad = gst_element_get_static_pad(pb->effects_bin, "sink");

  GstStructure* s = gst_structure_new_empty("enable_bypass");

  GstEvent* event = gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, s);

  gst_pad_send_event(sinkpad, event);

  gst_object_unref(sinkpad);
  gst_object_unref(srcpad);

  return GST_PAD_PROBE_OK;
}

}  // namespace

PipelineBase::PipelineBase(const std::string& tag, PipeManager* pipe_manager)
    : log_tag(tag),
      pm(pipe_manager),
      settings(g_settings_new("com.github.wwmm.pulseeffects")),
      spectrum_settings(g_settings_new("com.github.wwmm.pulseeffects.spectrum")),
      rtkit(std::make_unique<RealtimeKit>(tag)) {
  gst_init(nullptr, nullptr);

  pipeline = gst_pipeline_new("pipeline");

  bus = gst_element_get_bus(pipeline);

  gst_bus_enable_sync_message_emission(bus);
  gst_bus_add_signal_watch(bus);

  // bus callbacks

  g_signal_connect(bus, "message::error", G_CALLBACK(on_message_error), this);
  g_signal_connect(bus, "sync-message::stream-status", GCallback(on_stream_status), this);
  g_signal_connect(bus, "message::state-changed", G_CALLBACK(on_message_state_changed), this);
  g_signal_connect(bus, "message::latency", G_CALLBACK(on_message_latency), this);
  g_signal_connect(bus, "message::element", G_CALLBACK(on_message_element), this);

  // creating elements common to all pipelines

  gst_registry_scan_path(gst_registry_get(), PLUGINS_INSTALL_DIR);

  source = get_required_plugin("pipewiresrc", "source");
  queue_src = get_required_plugin("queue", nullptr);
  capsfilter = get_required_plugin("capsfilter", nullptr);
  sink = get_required_plugin("pipewiresink", "sink");
  spectrum = get_required_plugin("spectrum", "spectrum");
  global_level_meter = get_required_plugin("level", "global_level_meter");
  src_type = get_required_plugin("typefind", nullptr);

  init_spectrum_bin();
  init_effects_bin();

  // building the pipeline

  gst_bin_add_many(GST_BIN(pipeline), source, queue_src, capsfilter, src_type, effects_bin, spectrum_bin,
                   global_level_meter, sink, nullptr);

  gst_element_link_many(source, queue_src, capsfilter, src_type, effects_bin, spectrum_bin, global_level_meter, sink,
                        nullptr);

  // initializing properties

  g_object_set(source, "do-timestamp", 1, nullptr);

  g_object_set(queue_src, "silent", 1, nullptr);
  g_object_set(queue_src, "flush-on-eos", 1, nullptr);
  g_object_set(queue_src, "max-size-buffers", 0, nullptr);
  g_object_set(queue_src, "max-size-bytes", 0, nullptr);
  g_object_set(queue_src, "max-size-time", 0, nullptr);

  g_object_set(spectrum, "bands", spectrum_nbands, nullptr);
  g_object_set(spectrum, "threshold", spectrum_threshold, nullptr);

  g_signal_connect(src_type, "have-type", G_CALLBACK(on_src_type_changed), this);

  auto* sinkpad = gst_element_get_static_pad(sink, "sink");

  gst_pad_add_probe(sinkpad, GST_PAD_PROBE_TYPE_EVENT_UPSTREAM, on_sink_event, this, nullptr);

  g_object_unref(sinkpad);
}

PipelineBase::~PipelineBase() {
  timeout_connection.disconnect();

  set_null_pipeline();

  // Avoinding memory leak. If spectrum is not in a bin we have to unref it

  auto* s = gst_bin_get_by_name(GST_BIN(spectrum_bin), "spectrum");

  if (s == nullptr) {
    gst_object_unref(spectrum);
  }

  gst_object_unref(bus);
  gst_object_unref(pipeline);
  g_object_unref(settings);
  g_object_unref(spectrum_settings);
  g_object_unref(child_settings);
}

void PipelineBase::set_caps(const uint& sampling_rate) {
  this->sampling_rate = sampling_rate;

  auto caps_str = "audio/x-raw,format=F32LE,channels=2,rate=" + std::to_string(sampling_rate);

  auto* caps = gst_caps_from_string(caps_str.c_str());

  g_object_set(capsfilter, "caps", caps, nullptr);

  gst_caps_unref(caps);
}

void PipelineBase::init_spectrum_bin() {
  spectrum_bin = gst_bin_new("spectrum_bin");
  spectrum_identity_in = gst_element_factory_make("identity", nullptr);
  spectrum_identity_out = gst_element_factory_make("identity", nullptr);

  gst_bin_add_many(GST_BIN(spectrum_bin), spectrum_identity_in, spectrum_identity_out, nullptr);

  gst_element_link(spectrum_identity_in, spectrum_identity_out);

  auto* sinkpad = gst_element_get_static_pad(spectrum_identity_in, "sink");
  auto* srcpad = gst_element_get_static_pad(spectrum_identity_out, "src");

  gst_element_add_pad(spectrum_bin, gst_ghost_pad_new("sink", sinkpad));
  gst_element_add_pad(spectrum_bin, gst_ghost_pad_new("src", srcpad));

  g_object_unref(sinkpad);
  g_object_unref(srcpad);
}

void PipelineBase::init_global_level_meter_bin() {
  global_level_meter_bin = gst_bin_new("global_level_meter_bin");
  level_meter_identity_in = gst_element_factory_make("identity", nullptr);
  level_meter_identity_out = gst_element_factory_make("identity", nullptr);

  gst_bin_add_many(GST_BIN(global_level_meter_bin), level_meter_identity_in, level_meter_identity_out, nullptr);

  gst_element_link(level_meter_identity_in, level_meter_identity_out);

  auto* sinkpad = gst_element_get_static_pad(level_meter_identity_in, "sink");
  auto* srcpad = gst_element_get_static_pad(level_meter_identity_out, "src");

  gst_element_add_pad(global_level_meter_bin, gst_ghost_pad_new("sink", sinkpad));
  gst_element_add_pad(global_level_meter_bin, gst_ghost_pad_new("src", srcpad));

  g_object_unref(sinkpad);
  g_object_unref(srcpad);
}

void PipelineBase::init_effects_bin() {
  effects_bin = gst_bin_new("effects_bin");

  identity_in = gst_element_factory_make("identity", nullptr);
  identity_out = gst_element_factory_make("identity", nullptr);

  gst_bin_add_many(GST_BIN(effects_bin), identity_in, identity_out, nullptr);

  gst_element_link(identity_in, identity_out);

  auto* sinkpad = gst_element_get_static_pad(identity_in, "sink");
  auto* srcpad = gst_element_get_static_pad(identity_out, "src");

  gst_element_add_pad(effects_bin, gst_ghost_pad_new("sink", sinkpad));
  gst_element_add_pad(effects_bin, gst_ghost_pad_new("src", srcpad));

  g_object_unref(sinkpad);
  g_object_unref(srcpad);
}

void PipelineBase::set_input_node_id(const uint& id) const {
  auto path = std::to_string(id);

  g_object_set(source, "path", path.c_str(), nullptr);

  util::debug(log_tag + "using input device: " + path);
}

void PipelineBase::set_output_node_id(const uint& id) const {
  auto path = std::to_string(id);

  g_object_set(sink, "path", path.c_str(), nullptr);

  util::debug(log_tag + "using output device: " + path);
}

void PipelineBase::set_pulseaudio_props(const std::string& props) const {
  auto str = "props," + props;

  auto* s = gst_structure_from_string(str.c_str(), nullptr);

  g_object_set(source, "stream-properties", s, nullptr);
  g_object_set(sink, "stream-properties", s, nullptr);

  gst_structure_free(s);
}

void PipelineBase::set_null_pipeline() {
  gst_element_set_state(pipeline, GST_STATE_NULL);

  GstState state = GST_STATE_NULL;
  GstState pending = GST_STATE_NULL;

  gst_element_get_state(pipeline, &state, &pending, state_check_timeout);

  /* on_message_state is not called when going to null. I don't know why.
   * so we have to update the variable manually after setting to null.
   */

  if (state == GST_STATE_NULL) {
    playing = false;
  }

  util::debug(log_tag + gst_element_state_get_name(state) + " -> " + gst_element_state_get_name(pending));
}

auto PipelineBase::apps_want_to_play() -> bool {
  bool wants_to_play = false;

  for (const auto& a : apps_list) {
    if (a->wants_to_play) {
      wants_to_play = true;

      break;
    }
  }

  return wants_to_play;
}

void PipelineBase::update_pipeline_state() {
  bool wants_to_play = apps_want_to_play();

  GstState state = GST_STATE_NULL;
  GstState pending = GST_STATE_NULL;

  gst_element_get_state(pipeline, &state, &pending, state_check_timeout);

  if (state != GST_STATE_PLAYING && wants_to_play) {
    timeout_connection.disconnect();

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
  } else if (state == GST_STATE_PLAYING && !wants_to_play) {
    timeout_connection.disconnect();

    auto seconds = g_settings_get_int(settings, "audio-activity-timeout");

    timeout_connection = Glib::signal_timeout().connect_seconds(
        [=]() {
          GstState s = GST_STATE_NULL;
          GstState p = GST_STATE_NULL;

          gst_element_get_state(pipeline, &s, &p, state_check_timeout);

          if (s == GST_STATE_PLAYING && !apps_want_to_play()) {
            util::debug(log_tag + "No app wants to play audio. We will stop our pipeline.");

            gst_element_set_state(pipeline, GST_STATE_READY);
          }

          return false;
        },
        seconds);
  }
}

void PipelineBase::get_latency() {
  GstQuery* q = gst_query_new_latency();

  if (gst_element_query(pipeline, q) != 0) {
    gboolean live = 0;
    GstClockTime min = 0;
    GstClockTime max = 0;

    gst_query_parse_latency(q, &live, &min, &max);

    int latency = GST_TIME_AS_MSECONDS(min);

    util::debug(log_tag + "total latency: " + std::to_string(latency) + " ms");

    Glib::signal_idle().connect_once([=] { new_latency.emit(latency); });
  }

  gst_query_unref(q);
}

void PipelineBase::on_sink_changed(const std::shared_ptr<mySinkInfo>& sink_info) {
  // if (sink_info->name == "PulseEffects_apps") {
  //   if (sink_info->rate != sampling_rate) {
  //     gst_element_set_state(pipeline, GST_STATE_READY);

  //     set_caps(sink_info->rate);

  //     update_pipeline_state();
  //   }
  // }
}

void PipelineBase::on_source_changed(const std::shared_ptr<mySourceInfo>& source_info) {
  // if (source_info->name == "PulseEffects_mic.monitor") {
  //   if (source_info->rate != sampling_rate) {
  //     gst_element_set_state(pipeline, GST_STATE_READY);

  //     set_caps(source_info->rate);

  //     update_pipeline_state();
  //   }
  // }
}

void PipelineBase::init_spectrum() {
  g_signal_connect(spectrum_settings, "changed::n-points", G_CALLBACK(on_spectrum_n_points_changed), this);

  spectrum_freqs.clear();

  spectrum_start_index = 0U;

  for (uint n = 0U; n < spectrum_nbands; n++) {
    auto f = sampling_rate * (0.5 * n + 0.25) / spectrum_nbands;

    if (f > max_spectrum_freq) {
      break;
    }

    if (f > min_spectrum_freq) {
      spectrum_freqs.emplace_back(f);

      if (spectrum_start_index == 0U) {
        spectrum_start_index = n;
      }
    }
  }

  if (!spectrum_freqs.empty()) {
    spectrum_mag_tmp.resize(spectrum_freqs.size());

    auto npoints = g_settings_get_int(spectrum_settings, "n-points");

    spectrum_x_axis = util::logspace(log10(static_cast<float>(min_spectrum_freq)),
                                     log10(static_cast<float>(max_spectrum_freq)), npoints);

    spectrum_mag.resize(npoints);

    spline_f0 = spectrum_freqs[0];
    spline_df = spectrum_freqs[1] - spectrum_freqs[0];
  }
}

void PipelineBase::update_spectrum_interval(const double& value) const {
  const double one_second_in_ns = 1000000000.0;

  auto interval = static_cast<guint64>(one_second_in_ns / value);

  g_object_set(spectrum, "interval", interval, nullptr);
}

void PipelineBase::enable_spectrum() {
  auto* srcpad = gst_element_get_static_pad(spectrum_identity_in, "src");

  auto id = gst_pad_add_probe(
      srcpad, GST_PAD_PROBE_TYPE_IDLE,
      [](auto pad, auto info, auto d) {
        auto* l = static_cast<PipelineBase*>(d);

        auto* plugin = gst_bin_get_by_name(GST_BIN(l->spectrum_bin), "spectrum");

        if (!plugin) {
          gst_element_unlink(l->spectrum_identity_in, l->spectrum_identity_out);

          gst_bin_add(GST_BIN(l->spectrum_bin), l->spectrum);

          gst_element_link_many(l->spectrum_identity_in, l->spectrum, l->spectrum_identity_out, nullptr);

          gst_element_sync_state_with_parent(l->spectrum);

          util::debug(l->log_tag + "spectrum enabled");
        } else {
          util::debug(l->log_tag + "spectrum is already enabled");
        }

        return GST_PAD_PROBE_REMOVE;
      },
      this, nullptr);

  if (id != 0) {
    util::debug(log_tag + " spectrum will be enabled in another thread");
  }

  g_object_unref(srcpad);
}

void PipelineBase::disable_spectrum() {
  auto* srcpad = gst_element_get_static_pad(spectrum_identity_in, "src");

  auto id = gst_pad_add_probe(
      srcpad, GST_PAD_PROBE_TYPE_IDLE,
      [](auto pad, auto info, auto d) {
        auto* l = static_cast<PipelineBase*>(d);

        auto* plugin = gst_bin_get_by_name(GST_BIN(l->spectrum_bin), "spectrum");

        if (plugin) {
          gst_element_set_state(l->spectrum, GST_STATE_NULL);

          gst_element_unlink_many(l->spectrum_identity_in, l->spectrum, l->spectrum_identity_out, nullptr);

          gst_bin_remove(GST_BIN(l->spectrum_bin), l->spectrum);

          gst_element_link(l->spectrum_identity_in, l->spectrum_identity_out);

          util::debug(l->log_tag + "spectrum disabled");
        } else {
          util::debug(l->log_tag + "spectrum is already disabled");
        }

        return GST_PAD_PROBE_REMOVE;
      },
      this, nullptr);

  if (id != 0) {
    util::debug(log_tag + " spectrum will be disabled in another thread");
  }

  g_object_unref(srcpad);
}

auto PipelineBase::get_peak(GstMessage* message) -> std::array<double, 2> {
  std::array<double, 2> peak{0, 0};

  const GstStructure* s = gst_message_get_structure(message);

  auto* gpeak = (GValueArray*)g_value_get_boxed(gst_structure_get_value(s, "peak"));

  if (gpeak != nullptr) {
    if (gpeak->n_values == 2) {
      if (gpeak->values != nullptr) {
        peak[0] = g_value_get_double(gpeak->values);      // left
        peak[1] = g_value_get_double(gpeak->values + 1);  // right
      }
    }
  }

  return peak;
}

auto PipelineBase::get_required_plugin(const gchar* factoryname, const gchar* name) const -> GstElement* {
  GstElement* plugin = gst_element_factory_make(factoryname, name);

  if (plugin == nullptr) {
    throw std::runtime_error(log_tag + std::string("Failed to get required plugin: ") + factoryname);
  }

  return plugin;
}

void PipelineBase::do_bypass(const bool& value) {
  auto* srcpad = gst_element_get_static_pad(src_type, "src");

  if (value) {
    GstState state = GST_STATE_NULL;
    GstState pending = GST_STATE_NULL;

    gst_element_get_state(pipeline, &state, &pending, 0);

    if (state != GST_STATE_PLAYING) {
      gst_pad_add_probe(
          srcpad, GST_PAD_PROBE_TYPE_IDLE,
          [](auto pad, auto info, auto d) {
            on_bypass_on(d);

            return GST_PAD_PROBE_REMOVE;
          },
          this, nullptr);
    } else {
      gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, bypass_on_pad_blocked, this, nullptr);
    }
  } else {
    gst_pad_add_probe(
        srcpad, GST_PAD_PROBE_TYPE_IDLE,
        [](auto pad, auto info, auto d) {
          on_bypass_off(d);

          return GST_PAD_PROBE_REMOVE;
        },
        this, nullptr);
  }
}

auto PipelineBase::bypass_state() -> bool {
  auto* bin = gst_bin_get_by_name(GST_BIN(pipeline), "effects_bin");

  return bin == nullptr;
}
