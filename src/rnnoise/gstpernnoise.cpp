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

#include "gstpernnoise.hpp"
#include <gst/audio/audio.h>
#include "config.h"
#include "util.hpp"

GST_DEBUG_CATEGORY_STATIC(pernnoise_debug);
#define GST_CAT_DEFAULT (pernnoise_debug)

static void gst_pernnoise_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec);

static void gst_pernnoise_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec);

static GstFlowReturn gst_pernnoise_chain(GstPad* pad, GstObject* parent, GstBuffer* buffer);

static gboolean gst_pernnoise_sink_event(GstPad* pad, GstObject* parent, GstEvent* event);

static GstStateChangeReturn gst_pernnoise_change_state(GstElement* element, GstStateChange transition);

static void gst_pernnoise_process(GstPernnoise* pernnoise, GstBuffer* buffer);

static gboolean gst_pernnoise_src_query(GstPad* pad, GstObject* parent, GstQuery* query);

static void gst_pernnoise_finalize(GObject* object);

static GstFlowReturn gst_pernnoise_process(GstPernnoise* pernnoise);

static GstStaticPadTemplate sinktemplate =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

static GstStaticPadTemplate srctemplate =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

enum { PROP_BLOCKSIZE = 1 };

#define gst_pernnoise_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstPernnoise,
                        gst_pernnoise,
                        GST_TYPE_ELEMENT,
                        GST_DEBUG_CATEGORY_INIT(pernnoise_debug, "pernnoise", 0, "Pernnoise"));

static void gst_pernnoise_class_init(GstPernnoiseClass* klass) {
  GObjectClass* gobject_class;
  GstElementClass* gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)(klass);

  gobject_class->set_property = gst_pernnoise_set_property;
  gobject_class->get_property = gst_pernnoise_get_property;

  gst_element_class_add_static_pad_template(gstelement_class, &srctemplate);
  gst_element_class_add_static_pad_template(gstelement_class, &sinktemplate);

  gstelement_class->change_state = gst_pernnoise_change_state;

  gobject_class->finalize = gst_pernnoise_finalize;

  gst_element_class_set_static_metadata(gstelement_class, "Pernnoise element", "Filter",
                                        "Gives output buffers sizes that are a power of 2",
                                        "Wellington <wellingtonwallace@gmail.com>");
}

static void gst_pernnoise_init(GstPernnoise* pernnoise) {
  pernnoise->rate = -1;
  pernnoise->bpf = -1;
  pernnoise->blocksize = 480;  // for some reason I do not know rnnoise has to process buffers of 480 elements
  pernnoise->inbuf_n_samples = -1;
  pernnoise->flag_discont = false;
  pernnoise->adapter = gst_adapter_new();

  pernnoise->data_L.resize(pernnoise->blocksize);
  pernnoise->data_R.resize(pernnoise->blocksize);

  pernnoise->model = rnnoise_get_model("orig");

  if (pernnoise->model != nullptr) {
    pernnoise->state_left = rnnoise_create(pernnoise->model);
    pernnoise->state_right = rnnoise_create(pernnoise->model);

    auto attenuation = util::db_to_linear(-20.0F);

    rnnoise_set_param(pernnoise->state_left, RNNOISE_PARAM_MAX_ATTENUATION, attenuation);
    rnnoise_set_param(pernnoise->state_right, RNNOISE_PARAM_MAX_ATTENUATION, attenuation);
  }

  pernnoise->srcpad = gst_pad_new_from_static_template(&srctemplate, "src");

  /* configure event function on the pad before adding the pad to the element
   */

  gst_pad_set_query_function(pernnoise->srcpad, gst_pernnoise_src_query);

  gst_element_add_pad(GST_ELEMENT(pernnoise), pernnoise->srcpad);

  pernnoise->sinkpad = gst_pad_new_from_static_template(&sinktemplate, "sink");

  gst_pad_set_chain_function(pernnoise->sinkpad, GST_DEBUG_FUNCPTR(gst_pernnoise_chain));

  gst_pad_set_event_function(pernnoise->sinkpad, GST_DEBUG_FUNCPTR(gst_pernnoise_sink_event));

  gst_element_add_pad(GST_ELEMENT(pernnoise), pernnoise->sinkpad);
}

static void gst_pernnoise_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  GstPernnoise* pernnoise = GST_PERNNOISE(object);

  switch (prop_id) {
    case PROP_BLOCKSIZE: {
      pernnoise->blocksize = g_value_get_enum(value);

      gst_element_post_message(GST_ELEMENT_CAST(pernnoise), gst_message_new_latency(GST_OBJECT_CAST(pernnoise)));

      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);

      break;
  }
}

static void gst_pernnoise_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  GstPernnoise* pernnoise = GST_PERNNOISE(object);

  switch (prop_id) {
    case PROP_BLOCKSIZE:
      g_value_set_enum(value, pernnoise->blocksize);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static GstFlowReturn gst_pernnoise_chain(GstPad* pad, GstObject* parent, GstBuffer* buffer) {
  GstPernnoise* pernnoise = GST_PERNNOISE(parent);
  GstFlowReturn ret = GST_FLOW_OK;

  if (GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_DISCONT)) {
    gst_adapter_clear(pernnoise->adapter);

    pernnoise->inbuf_n_samples = -1;

    pernnoise->flag_discont = true;
  }

  if (pernnoise->inbuf_n_samples == -1) {
    GstMapInfo map;

    gst_buffer_map(buffer, &map, GST_MAP_READ);

    pernnoise->inbuf_n_samples = map.size / pernnoise->bpf;

    util::debug("pernnoise: input block size " + std::to_string(pernnoise->inbuf_n_samples) + " frames");

    util::debug("pernnoise: we will read in chunks of " + std::to_string(pernnoise->blocksize) + " frames");

    gst_buffer_unmap(buffer, &map);
  }

  gst_adapter_push(pernnoise->adapter, buffer);

  ret = gst_pernnoise_process(pernnoise);

  return ret;
}

static GstFlowReturn gst_pernnoise_process(GstPernnoise* pernnoise) {
  GstFlowReturn ret = GST_FLOW_OK;

  gsize nbytes = pernnoise->blocksize * pernnoise->bpf;
  auto duration = GST_FRAMES_TO_CLOCK_TIME(pernnoise->blocksize, pernnoise->rate);

  while (gst_adapter_available(pernnoise->adapter) > nbytes && (ret == GST_FLOW_OK)) {
    GstBuffer* b = gst_adapter_take_buffer(pernnoise->adapter, nbytes);

    if (b != nullptr) {
      b = gst_buffer_make_writable(b);

      gst_pernnoise_process(pernnoise, b);

      GST_BUFFER_OFFSET(b) = gst_adapter_prev_offset(pernnoise->adapter, nullptr);
      GST_BUFFER_PTS(b) = gst_adapter_prev_pts(pernnoise->adapter, nullptr);
      GST_BUFFER_DURATION(b) = duration;

      if (pernnoise->flag_discont) {
        gst_buffer_set_flags(b, GST_BUFFER_FLAG_DISCONT);
        gst_buffer_set_flags(b, GST_BUFFER_FLAG_RESYNC);

        pernnoise->flag_discont = false;
      } else {
        gst_buffer_unset_flags(b, GST_BUFFER_FLAG_DISCONT);
      }

      gst_buffer_set_flags(b, GST_BUFFER_FLAG_NON_DROPPABLE);
      gst_buffer_set_flags(b, GST_BUFFER_FLAG_LIVE);

      ret = gst_pad_push(pernnoise->srcpad, b);
    }
  }

  return ret;
}

static gboolean gst_pernnoise_sink_event(GstPad* pad, GstObject* parent, GstEvent* event) {
  GstPernnoise* pernnoise = GST_PERNNOISE(parent);
  gboolean ret = 1;

  switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS: {
      GstCaps* caps = nullptr;
      GstAudioInfo info;

      gst_event_parse_caps(event, &caps);

      gst_audio_info_from_caps(&info, caps);

      pernnoise->rate = GST_AUDIO_INFO_RATE(&info);
      pernnoise->bpf = GST_AUDIO_INFO_BPF(&info);

      rnnoise_set_param(pernnoise->state_left, RNNOISE_PARAM_SAMPLE_RATE, static_cast<float>(pernnoise->rate));
      rnnoise_set_param(pernnoise->state_right, RNNOISE_PARAM_SAMPLE_RATE, static_cast<float>(pernnoise->rate));

      /* push the event downstream */

      ret = gst_pad_push_event(pernnoise->srcpad, event);

      break;
    }
    case GST_EVENT_FLUSH_START: {
      gst_pernnoise_process(pernnoise);
      gst_adapter_clear(pernnoise->adapter);

      pernnoise->inbuf_n_samples = -1;

      ret = gst_pad_push_event(pernnoise->srcpad, event);

      break;
    }
    case GST_EVENT_EOS: {
      gst_pernnoise_process(pernnoise);
      gst_adapter_clear(pernnoise->adapter);

      pernnoise->inbuf_n_samples = -1;

      ret = gst_pad_push_event(pernnoise->srcpad, event);

      break;
    }
    default:
      ret = gst_pad_push_event(pernnoise->srcpad, event);
      break;
  }

  return ret;
}

static GstStateChangeReturn gst_pernnoise_change_state(GstElement* element, GstStateChange transition) {
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstPernnoise* pernnoise = GST_PERNNOISE(element);

  /*up changes*/

  // switch (transition) {
  //   case GST_STATE_CHANGE_NULL_TO_READY:
  //     break;
  //   default:
  //     break;
  // }

  /*down changes*/

  ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

  if (ret == GST_STATE_CHANGE_FAILURE) {
    return ret;
  }

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY: {
      gst_adapter_clear(pernnoise->adapter);

      pernnoise->inbuf_n_samples = -1;

      break;
    }
    default:
      break;
  }

  return ret;
}

static void gst_pernnoise_process(GstPernnoise* pernnoise, GstBuffer* buffer) {
  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  auto* data = reinterpret_cast<float*>(map.data);

  // deinterleave
  for (int n = 0U; n < pernnoise->blocksize; n++) {
    pernnoise->data_L[n] = data[2U * n] * SHRT_MAX;
    pernnoise->data_R[n] = data[2U * n + 1U] * SHRT_MAX;
  }

  rnnoise_process_frame(pernnoise->state_left, pernnoise->data_L.data(), pernnoise->data_L.data());
  rnnoise_process_frame(pernnoise->state_right, pernnoise->data_R.data(), pernnoise->data_R.data());

  // interleave
  for (int n = 0U; n < pernnoise->blocksize; n++) {
    data[2U * n] = pernnoise->data_L[n] / SHRT_MAX;
    data[2U * n + 1U] = pernnoise->data_R[n] / SHRT_MAX;
  }

  gst_buffer_unmap(buffer, &map);
}

static gboolean gst_pernnoise_src_query(GstPad* pad, GstObject* parent, GstQuery* query) {
  GstPernnoise* pernnoise = GST_PERNNOISE(parent);
  bool ret = true;

  switch (GST_QUERY_TYPE(query)) {
    case GST_QUERY_LATENCY: {
      if (pernnoise->rate > 0) {
        ret = gst_pad_peer_query(pernnoise->sinkpad, query);

        if (ret && pernnoise->inbuf_n_samples != -1 && pernnoise->inbuf_n_samples < pernnoise->blocksize) {
          GstClockTime min, max;
          gboolean live;
          guint64 latency;

          gst_query_parse_latency(query, &live, &min, &max);

          /* add our own latency */

          latency = gst_util_uint64_scale_round(pernnoise->blocksize - pernnoise->inbuf_n_samples, GST_SECOND,
                                                pernnoise->rate);

          // std::cout << "latency: " << latency << std::endl;
          // std::cout << "n: " << pernnoise->inbuf_n_samples
          //           << std::endl;

          min += latency;

          if (max != GST_CLOCK_TIME_NONE) {
            max += latency;
          }

          // std::cout << min << "\t" << max << "\t" << live
          //           << std::endl;

          gst_query_set_latency(query, live, min, max);
        }

      } else {
        ret = false;
      }

      break;
    }
    default:
      /* just call the default handler */
      ret = gst_pad_query_default(pad, parent, query);
      break;
  }

  return ret;
}

void gst_pernnoise_finalize(GObject* object) {
  GstPernnoise* pernnoise = GST_PERNNOISE(object);

  GST_DEBUG_OBJECT(pernnoise, "finalize");

  gst_adapter_clear(pernnoise->adapter);
  g_object_unref(pernnoise->adapter);

  /* clean up object here */

  G_OBJECT_CLASS(gst_pernnoise_parent_class)->finalize(object);
}

static gboolean plugin_init(GstPlugin* plugin) {
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "pernnoise", GST_RANK_NONE, GST_TYPE_PERNNOISE);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  pernnoise,
                  "PulseEffects Buffer Adapter",
                  plugin_init,
                  VERSION,
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
