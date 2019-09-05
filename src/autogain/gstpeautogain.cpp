/**
 * SECTION:element-gstpeautogain
 *
 * The peautogain element does auto volume and is based on libebur128.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v audiotestsrc ! peautogain ! pulsesink
 * ]|
 * The peautogain element does auto volume and is based on libebur128.
 * </refsect2>
 */

#include "gstpeautogain.hpp"
#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <cmath>
#include <iostream>
#include "config.h"

GST_DEBUG_CATEGORY_STATIC(gst_peautogain_debug_category);
#define GST_CAT_DEFAULT gst_peautogain_debug_category

/* prototypes */

static void gst_peautogain_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);

static void gst_peautogain_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);

static gboolean gst_peautogain_setup(GstAudioFilter* filter, const GstAudioInfo* info);

static GstFlowReturn gst_peautogain_transform_ip(GstBaseTransform* trans, GstBuffer* buffer);

static void gst_peautogain_finalize(GObject* object);

static void gst_peautogain_setup_ebur(GstPeautogain* peautogain);

static void gst_peautogain_reset(GstPeautogain* peautogain);

static void gst_peautogain_process(GstPeautogain* peautogain, GstBuffer* buffer);

enum {
  PROP_TARGET = 1,
  PROP_WEIGHT_M,
  PROP_WEIGHT_S,
  PROP_WEIGHT_I,
  PROP_M,
  PROP_S,
  PROP_I,
  PROP_R,
  PROP_L,
  PROP_G,
  PROP_LRA,
  PROP_NOTIFY,
  PROP_DETECT_SILENCE,
  PROP_RESET
};

/* pad templates */

static GstStaticPadTemplate gst_peautogain_src_template =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

static GstStaticPadTemplate gst_peautogain_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstPeautogain,
    gst_peautogain,
    GST_TYPE_AUDIO_FILTER,
    GST_DEBUG_CATEGORY_INIT(gst_peautogain_debug_category, "peautogain", 0, "debug category for peautogain element"));

static void gst_peautogain_class_init(GstPeautogainClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

  GstAudioFilterClass* audio_filter_class = GST_AUDIO_FILTER_CLASS(klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */

  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_peautogain_src_template);
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_peautogain_sink_template);

  gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass), "PulseEffects ebur128 level meter", "Generic",
                                        "PulseEffects ebur128 level meter", "Wellington <wellingtonwallace@gmail.com>");

  /* define virtual function pointers */

  gobject_class->set_property = gst_peautogain_set_property;
  gobject_class->get_property = gst_peautogain_get_property;
  gobject_class->finalize = gst_peautogain_finalize;

  audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_peautogain_setup);
  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR(gst_peautogain_transform_ip);
  base_transform_class->transform_ip_on_passthrough = false;

  /* define properties */

  g_object_class_install_property(
      gobject_class, PROP_TARGET,
      g_param_spec_float("target", "Target Level", "Target loudness level (in LUFS)", -100.0f, 0.0f, -23.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_WEIGHT_M,
      g_param_spec_int("weight-m", "Weight 0", "Momentary loudness weight", 0, 100, 1,
                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_WEIGHT_S,
      g_param_spec_int("weight-s", "Weight 1", "Short term loudness weight", 0, 100, 1,
                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_WEIGHT_I,
      g_param_spec_int("weight-i", "Weight 2", "Integrated loudness weight", 0, 100, 1,
                       static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_M,
      g_param_spec_float("m", "Momentary Level", "Momentary loudness level (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_S,
      g_param_spec_float("s", "Short Term Level", "Short term loudness level (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_I,
      g_param_spec_float("i", "Integrated Level", "Integrated loudness level (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_R,
      g_param_spec_float("r", "Relative Level", "Relative threshold level (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_L,
      g_param_spec_float("l", "Loudness Level", "Estimated Loudness level (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_G,
      g_param_spec_float("g", "Gain", "Correction gain", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_NOTIFY,
      g_param_spec_boolean("notify-host", "Notify Host", "Notify host of variable changes", true,
                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_LRA,
      g_param_spec_float("lra", "Loudness Range", "Loudness Range (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_DETECT_SILENCE,
      g_param_spec_boolean("detect-silence", "Detect Silence",
                           "Do not change gain if the momentary term is below the relative loudness", true,
                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_RESET,
      g_param_spec_boolean("reset", "Reset History", "Completely reset the library ebur128 state", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
}

static void gst_peautogain_init(GstPeautogain* peautogain) {
  peautogain->ready = false;
  peautogain->bpf = 0;
  peautogain->rate = 0;
  peautogain->target = -23.0f;  // LUFS
  peautogain->weight_m = 1;
  peautogain->weight_s = 1;
  peautogain->weight_i = 1;
  peautogain->momentary = 0.0f;
  peautogain->shortterm = 0.0f;
  peautogain->global = 0.0f;
  peautogain->relative = 0.0f;
  peautogain->loudness = 0.0f;
  peautogain->gain = 1.0f;
  peautogain->range = 0.0f;
  peautogain->notify_samples = 0;
  peautogain->sample_count = 0;
  peautogain->notify = true;
  peautogain->detect_silence = true;
  peautogain->reset = false;
  peautogain->ebur_state = nullptr;

  gst_base_transform_set_in_place(GST_BASE_TRANSFORM(peautogain), true);
}

void gst_peautogain_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec) {
  GstPeautogain* peautogain = GST_PEAUTOGAIN(object);

  GST_DEBUG_OBJECT(peautogain, "set_property");

  switch (property_id) {
    case PROP_TARGET:
      peautogain->target = g_value_get_float(value);
      break;
    case PROP_WEIGHT_M:
      peautogain->weight_m = g_value_get_int(value);
      break;
    case PROP_WEIGHT_S:
      peautogain->weight_s = g_value_get_int(value);
      break;
    case PROP_WEIGHT_I:
      peautogain->weight_i = g_value_get_int(value);
      break;
    case PROP_NOTIFY:
      peautogain->notify = g_value_get_boolean(value);
      break;
    case PROP_DETECT_SILENCE:
      peautogain->detect_silence = g_value_get_boolean(value);
      break;
    case PROP_RESET:
      peautogain->reset = g_value_get_boolean(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

void gst_peautogain_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec) {
  GstPeautogain* peautogain = GST_PEAUTOGAIN(object);

  GST_DEBUG_OBJECT(peautogain, "get_property");

  switch (property_id) {
    case PROP_TARGET:
      g_value_set_float(value, peautogain->target);
      break;
    case PROP_WEIGHT_M:
      g_value_set_int(value, peautogain->weight_m);
      break;
    case PROP_WEIGHT_S:
      g_value_set_int(value, peautogain->weight_s);
      break;
    case PROP_WEIGHT_I:
      g_value_set_int(value, peautogain->weight_i);
      break;
    case PROP_M:
      g_value_set_float(value, peautogain->momentary);
      break;
    case PROP_S:
      g_value_set_float(value, peautogain->shortterm);
      break;
    case PROP_I:
      g_value_set_float(value, peautogain->global);
      break;
    case PROP_R:
      g_value_set_float(value, peautogain->relative);
      break;
    case PROP_L:
      g_value_set_float(value, peautogain->loudness);
      break;
    case PROP_G:
      g_value_set_float(value, peautogain->gain);
      break;
    case PROP_LRA:
      g_value_set_float(value, peautogain->range);
      break;
    case PROP_NOTIFY:
      g_value_set_boolean(value, peautogain->notify);
      break;
    case PROP_DETECT_SILENCE:
      g_value_set_boolean(value, peautogain->detect_silence);
      break;
    case PROP_RESET:
      g_value_set_boolean(value, peautogain->reset);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static gboolean gst_peautogain_setup(GstAudioFilter* filter, const GstAudioInfo* info) {
  GstPeautogain* peautogain = GST_PEAUTOGAIN(filter);

  GST_DEBUG_OBJECT(peautogain, "setup");

  std::lock_guard<std::mutex> lock(peautogain->lock_guard_ebu);

  peautogain->bpf = info->bpf;
  peautogain->rate = info->rate;
  peautogain->notify_samples = GST_CLOCK_TIME_TO_FRAMES(GST_SECOND / 10, info->rate);  // notify every 0.1 seconds

  gst_peautogain_setup_ebur(peautogain);

  return true;
}

static GstFlowReturn gst_peautogain_transform_ip(GstBaseTransform* trans, GstBuffer* buffer) {
  GstPeautogain* peautogain = GST_PEAUTOGAIN(trans);

  GST_DEBUG_OBJECT(peautogain, "transform");

  std::lock_guard<std::mutex> lock(peautogain->lock_guard_ebu);

  if (peautogain->reset) {
    gst_peautogain_reset(peautogain);
  }

  if (peautogain->ready) {
    gst_peautogain_process(peautogain, buffer);
  } else {
    gst_peautogain_setup_ebur(peautogain);
  }

  return GST_FLOW_OK;
}

void gst_peautogain_finalize(GObject* object) {
  GstPeautogain* peautogain = GST_PEAUTOGAIN(object);

  GST_DEBUG_OBJECT(peautogain, "finalize");

  std::lock_guard<std::mutex> lock(peautogain->lock_guard_ebu);

  gst_peautogain_reset(peautogain);

  G_OBJECT_CLASS(gst_peautogain_parent_class)->finalize(object);
}

static void gst_peautogain_setup_ebur(GstPeautogain* peautogain) {
  if (!peautogain->ready) {
    peautogain->ebur_state = ebur128_init(
        2, peautogain->rate,
        EBUR128_MODE_S | EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_SAMPLE_PEAK | EBUR128_MODE_HISTOGRAM);

    ebur128_set_channel(peautogain->ebur_state, 0, EBUR128_LEFT);
    ebur128_set_channel(peautogain->ebur_state, 1, EBUR128_RIGHT);

    ebur128_set_max_history(peautogain->ebur_state, 30 * 1000);  // ms

    peautogain->ready = true;
  }
}

static void gst_peautogain_reset(GstPeautogain* peautogain) {
  peautogain->ready = false;
  peautogain->reset = false;
  peautogain->gain = 1.0f;

  if (peautogain->ebur_state != nullptr) {
    ebur128_destroy(&peautogain->ebur_state);

    peautogain->ebur_state = nullptr;
  }
}

static void gst_peautogain_process(GstPeautogain* peautogain, GstBuffer* buffer) {
  GstMapInfo map;
  double momentary, shortterm, global, relative, range;
  bool failed = false;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  float* data = (float*)map.data;

  guint num_samples = map.size / peautogain->bpf;

  ebur128_add_frames_float(peautogain->ebur_state, data, num_samples);

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(peautogain->ebur_state, &momentary)) {
    failed = true;
  } else {
    peautogain->momentary = (float)momentary;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(peautogain->ebur_state, &shortterm)) {
    failed = true;
  } else {
    peautogain->shortterm = (float)shortterm;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(peautogain->ebur_state, &global)) {
    failed = true;
  } else {
    peautogain->global = (float)global;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(peautogain->ebur_state, &relative)) {
    failed = true;
  } else {
    peautogain->relative = (float)relative;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(peautogain->ebur_state, &range)) {
    failed = true;
  } else {
    peautogain->range = (float)range;
  }

  bool playing_silence = (peautogain->momentary < peautogain->relative && peautogain->detect_silence) ? true : false;

  if (peautogain->relative > -70 && !failed && !playing_silence) {
    double peak_L, peak_R;

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(peautogain->ebur_state, 0, &peak_L)) {
      failed = true;
    }

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(peautogain->ebur_state, 1, &peak_R)) {
      failed = true;
    }

    if (!failed) {
      peautogain->loudness =
          (peautogain->weight_m * peautogain->momentary + peautogain->weight_s * peautogain->shortterm +
           peautogain->weight_i * peautogain->global) /
          (peautogain->weight_m + peautogain->weight_s + peautogain->weight_i);

      float diff = peautogain->target - peautogain->loudness;

      // 10^(diff/20). The way below should be faster than using pow
      float gain = expf((diff / 20.0f) * logf(10.0f));

      float peak = (peak_L > peak_R) ? peak_L : peak_R;

      if (gain * peak < 1.0f) {
        peautogain->gain = gain;
      }
    }
  }

  for (unsigned int n = 0; n < 2 * num_samples; n++) {
    data[n] = data[n] * peautogain->gain;
  }

  gst_buffer_unmap(buffer, &map);

  if (!failed && peautogain->notify) {
    peautogain->sample_count += num_samples;

    if (peautogain->sample_count >= peautogain->notify_samples) {
      peautogain->sample_count = 0;

      g_object_notify(G_OBJECT(peautogain), "m");
      g_object_notify(G_OBJECT(peautogain), "s");
      g_object_notify(G_OBJECT(peautogain), "i");
      g_object_notify(G_OBJECT(peautogain), "r");
      g_object_notify(G_OBJECT(peautogain), "l");
      g_object_notify(G_OBJECT(peautogain), "lra");
      g_object_notify(G_OBJECT(peautogain), "g");
    }
  }
}

static gboolean plugin_init(GstPlugin* plugin) {
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "peautogain", GST_RANK_NONE, GST_TYPE_PEAUTOGAIN);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  peautogain,
                  "PulseEffects autogain based on libebur128",
                  plugin_init,
                  VERSION,
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
