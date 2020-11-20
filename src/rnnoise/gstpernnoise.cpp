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

/**
 * SECTION:element-gstpernnoise
 *
 * The pernnoise uses the rnnoise library to remove background noise from audio.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v audiotestsrc ! pernnoise ! pulsesink
 * ]|
 * The pernnoise uses the rnnoise library to remove background noise from audio.
 * </refsect2>
 */

#include "gstpernnoise.hpp"
#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <cstring>
#include <mutex>
#include "config.h"
#include "util.hpp"

std::mutex rnnoise_mutex;

GST_DEBUG_CATEGORY_STATIC(gst_pernnoise_debug_category);
#define GST_CAT_DEFAULT gst_pernnoise_debug_category

/* prototypes */

static void gst_pernnoise_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);

static void gst_pernnoise_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);

static void gst_pernnoise_finalize(GObject* object);

static gboolean gst_pernnoise_setup(GstAudioFilter* filter, const GstAudioInfo* info);

static GstFlowReturn gst_pernnoise_transform_ip(GstBaseTransform* trans, GstBuffer* buffer);

static gboolean gst_pernnoise_stop(GstBaseTransform* base);

static void gst_pernnoise_set_model_name(GstPernnoise* pernnoise, gchar* value);

static void gst_pernnoise_set_model_path(GstPernnoise* pernnoise, gchar* value);

static void gst_pernnoise_process(GstPernnoise* pernnoise, GstBuffer* buffer);

static void gst_pernnoise_setup_rnnoise(GstPernnoise* pernnoise);

static void gst_pernnoise_finish_rnnoise(GstPernnoise* pernnoise);

/*global variables and my defines*/

// taken from https://github.com/x42/convoLV2/blob/master/convolution.cc
/*
 * Priority should match -P parameter passed to jackd.
 * Sched.class: either SCHED_FIFO or SCHED_RR (I think Jack uses SCHED_FIFO).
 *
 * THREAD_SYNC_MODE must be true if you want to use the plugin in Jack
 * freewheeling mode (eg. while exporting in Ardour). You may only use
 * false if you *only* run the plugin realtime.
 */
#define CONVPROC_SCHEDULER_PRIORITY 0
#define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
#define THREAD_SYNC_MODE true

enum { PROP_MODEL_NAME = 1, PROP_MODEL_PATH };

/* pad templates */

static GstStaticPadTemplate gst_pernnoise_src_template =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

static GstStaticPadTemplate gst_pernnoise_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstPernnoise,
    gst_pernnoise,
    GST_TYPE_AUDIO_FILTER,
    GST_DEBUG_CATEGORY_INIT(gst_pernnoise_debug_category, "pernnoise", 0, "debug category for pernnoise element"));

static void gst_pernnoise_class_init(GstPernnoiseClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

  GstAudioFilterClass* audio_filter_class = GST_AUDIO_FILTER_CLASS(klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */

  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_pernnoise_src_template);
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_pernnoise_sink_template);

  gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass), "PulseEffects Convolver", "Generic",
                                        "PulseEffects Convolver", "Wellington <wellingtonwallace@gmail.com>");

  /* define virtual function pointers */

  gobject_class->set_property = gst_pernnoise_set_property;
  gobject_class->get_property = gst_pernnoise_get_property;

  gobject_class->finalize = gst_pernnoise_finalize;

  audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_pernnoise_setup);

  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR(gst_pernnoise_transform_ip);

  base_transform_class->transform_ip_on_passthrough = false;

  base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_pernnoise_stop);

  /* define properties */

  g_object_class_install_property(
      gobject_class, PROP_MODEL_NAME,
      g_param_spec_string("model-name", "Model Name", "Name of the built-in model", nullptr,
                          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_MODEL_PATH,
      g_param_spec_string("model-path", "Model Path", "Path of the model file", nullptr,
                          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
}

static void gst_pernnoise_init(GstPernnoise* pernnoise) {
  pernnoise->rate = -1;
  pernnoise->ready = false;
  pernnoise->bpf = -1;
  pernnoise->inbuf_n_samples = -1;
  pernnoise->blocksize = 480;  // for some reason I do not know rnnoise has to process buffers of 480 elements

  pernnoise->data_L.resize(pernnoise->blocksize);
  pernnoise->data_R.resize(pernnoise->blocksize);

  gst_base_transform_set_in_place(GST_BASE_TRANSFORM(pernnoise), true);
}

void gst_pernnoise_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec) {
  GstPernnoise* pernnoise = GST_PERNNOISE(object);

  GST_DEBUG_OBJECT(pernnoise, "set_property");

  switch (property_id) {
    case PROP_MODEL_NAME: {
      gst_pernnoise_set_model_name(pernnoise, g_value_dup_string(value));

      break;
    }
    case PROP_MODEL_PATH: {
      gst_pernnoise_set_model_path(pernnoise, g_value_dup_string(value));

      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);

      break;
  }
}

void gst_pernnoise_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec) {
  GstPernnoise* pernnoise = GST_PERNNOISE(object);

  GST_DEBUG_OBJECT(pernnoise, "get_property");

  switch (property_id) {
    case PROP_MODEL_NAME:
      g_value_set_string(value, pernnoise->model_name);
      break;
    case PROP_MODEL_PATH:
      g_value_set_string(value, pernnoise->model_path);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

void gst_pernnoise_finalize(GObject* object) {
  GstPernnoise* pernnoise = GST_PERNNOISE(object);

  GST_DEBUG_OBJECT(pernnoise, "finalize");

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  gst_pernnoise_finish_rnnoise(pernnoise);

  /* clean up object here */

  G_OBJECT_CLASS(gst_pernnoise_parent_class)->finalize(object);
}

static gboolean gst_pernnoise_setup(GstAudioFilter* filter, const GstAudioInfo* info) {
  GstPernnoise* pernnoise = GST_PERNNOISE(filter);

  GST_DEBUG_OBJECT(pernnoise, "setup");

  pernnoise->rate = info->rate;
  pernnoise->bpf = GST_AUDIO_INFO_BPF(info);

  /*
  this function is called whenever there is a format change. So we reinitialize rnnoise.
  */

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  gst_pernnoise_finish_rnnoise(pernnoise);

  return true;
}

static GstFlowReturn gst_pernnoise_transform_ip(GstBaseTransform* trans, GstBuffer* buffer) {
  GstPernnoise* pernnoise = GST_PERNNOISE(trans);

  GST_DEBUG_OBJECT(pernnoise, "transform");

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  if (pernnoise->ready) {
    gst_pernnoise_process(pernnoise, buffer);
  } else {
    gst_pernnoise_setup_rnnoise(pernnoise);
  }

  return GST_FLOW_OK;
}

static gboolean gst_pernnoise_stop(GstBaseTransform* base) {
  GstPernnoise* pernnoise = GST_PERNNOISE(base);

  std::lock_guard<std::mutex> guard(rnnoise_mutex);

  gst_pernnoise_finish_rnnoise(pernnoise);

  return true;
}

static void gst_pernnoise_set_model_name(GstPernnoise* pernnoise, gchar* value) {
  if (value != nullptr) {
    if (pernnoise->model_name != nullptr) {
      if (std::strcmp(value, pernnoise->model_name) != 0) {
        g_free(pernnoise->model_name);

        pernnoise->model_name = value;

        std::lock_guard<std::mutex> guard(rnnoise_mutex);

        gst_pernnoise_finish_rnnoise(pernnoise);
      }
    } else {
      // plugin is being initialized

      g_free(pernnoise->model_name);

      pernnoise->model_name = value;
    }
  }
}

static void gst_pernnoise_set_model_path(GstPernnoise* pernnoise, gchar* value) {
  if (value != nullptr) {
    if (pernnoise->model_path != nullptr) {
      if (std::strcmp(value, pernnoise->model_path) != 0) {
        g_free(pernnoise->model_path);

        pernnoise->model_path = value;

        std::lock_guard<std::mutex> guard(rnnoise_mutex);

        gst_pernnoise_finish_rnnoise(pernnoise);
      }
    } else {
      // plugin is being initialized

      g_free(pernnoise->model_path);

      pernnoise->model_path = value;
    }
  }
}

static void gst_pernnoise_setup_rnnoise(GstPernnoise* pernnoise) {
  FILE* f = fopen(pernnoise->model_path, "r");

  if (f != nullptr) {
    util::debug("rnnoise plugin: loading model from file: " + std::string(pernnoise->model_path));

    pernnoise->model = rnnoise_model_from_file(f);

    fclose(f);
  }

  if (pernnoise->model != nullptr) {
    pernnoise->state_left = rnnoise_create(pernnoise->model);
    pernnoise->state_right = rnnoise_create(pernnoise->model);

    pernnoise->ready = true;
  } else {
    pernnoise->ready = false;

    util::debug("could not open the model file: " + std::string(pernnoise->model_path));
  }
}

static void gst_pernnoise_process(GstPernnoise* pernnoise, GstBuffer* buffer) {
  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  auto* data = reinterpret_cast<float*>(map.data);

  // deinterleave
  for (int n = 0U; n < pernnoise->blocksize; n++) {
    pernnoise->data_L[n] = data[2U * n] * (SHRT_MAX + 1);
    pernnoise->data_R[n] = data[2U * n + 1U] * (SHRT_MAX + 1);
  }

  rnnoise_process_frame(pernnoise->state_left, pernnoise->data_L.data(), pernnoise->data_L.data());
  rnnoise_process_frame(pernnoise->state_right, pernnoise->data_R.data(), pernnoise->data_R.data());

  // interleave
  for (int n = 0U; n < pernnoise->blocksize; n++) {
    data[2U * n] = pernnoise->data_L[n] / (SHRT_MAX + 1);
    data[2U * n + 1U] = pernnoise->data_R[n] / (SHRT_MAX + 1);
  }

  gst_buffer_unmap(buffer, &map);
}

static void gst_pernnoise_finish_rnnoise(GstPernnoise* pernnoise) {
  if (pernnoise->ready) {
    pernnoise->ready = false;

    rnnoise_destroy(pernnoise->state_left);
    rnnoise_destroy(pernnoise->state_right);
    rnnoise_model_free(pernnoise->model);

    pernnoise->state_left = nullptr;
    pernnoise->state_right = nullptr;
    pernnoise->model = nullptr;
  }
}

static gboolean plugin_init(GstPlugin* plugin) {
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "pernnoise", GST_RANK_NONE, GST_TYPE_PERNNOISE);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  pernnoise,
                  "PulseEffects Noise Remover",
                  plugin_init,
                  VERSION,
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
