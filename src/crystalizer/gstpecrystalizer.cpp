/**
 * SECTION:element-gstpecrystalizer
 *
 * The pecrystalizer element increases dynamic range.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v audiotestsrc ! pecrystalizer ! pulsesink
 * ]|
 * The pecrystalizer element increases dynamic range.
 * </refsect2>
 */

#include "gstpecrystalizer.hpp"
#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <algorithm>
#include <cmath>
#include "config.h"

GST_DEBUG_CATEGORY_STATIC(gst_pecrystalizer_debug_category);
#define GST_CAT_DEFAULT gst_pecrystalizer_debug_category

/* prototypes */

static void gst_pecrystalizer_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);

static void gst_pecrystalizer_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);

static gboolean gst_pecrystalizer_setup(GstAudioFilter* filter, const GstAudioInfo* info);

static GstFlowReturn gst_pecrystalizer_transform_ip(GstBaseTransform* trans, GstBuffer* buffer);

static gboolean gst_pecrystalizer_stop(GstBaseTransform* base);

static void gst_pecrystalizer_setup_filters(GstPecrystalizer* pecrystalizer);

static void gst_pecrystalizer_process(GstPecrystalizer* pecrystalizer, GstBuffer* buffer);

static gboolean gst_pecrystalizer_src_query(GstPad* pad, GstObject* parent, GstQuery* query);

static void gst_pecrystalizer_finish_filters(GstPecrystalizer* pecrystalizer);

static void gst_pecrystalizer_finalize(GObject* object);

enum {
  PROP_INTENSITY_BAND0 = 1,
  PROP_INTENSITY_BAND1,
  PROP_INTENSITY_BAND2,
  PROP_INTENSITY_BAND3,
  PROP_INTENSITY_BAND4,
  PROP_INTENSITY_BAND5,
  PROP_INTENSITY_BAND6,
  PROP_INTENSITY_BAND7,
  PROP_INTENSITY_BAND8,
  PROP_INTENSITY_BAND9,
  PROP_INTENSITY_BAND10,
  PROP_INTENSITY_BAND11,
  PROP_INTENSITY_BAND12,
  PROP_MUTE_BAND0,
  PROP_MUTE_BAND1,
  PROP_MUTE_BAND2,
  PROP_MUTE_BAND3,
  PROP_MUTE_BAND4,
  PROP_MUTE_BAND5,
  PROP_MUTE_BAND6,
  PROP_MUTE_BAND7,
  PROP_MUTE_BAND8,
  PROP_MUTE_BAND9,
  PROP_MUTE_BAND10,
  PROP_MUTE_BAND11,
  PROP_MUTE_BAND12,
  PROP_BYPASS_BAND0,
  PROP_BYPASS_BAND1,
  PROP_BYPASS_BAND2,
  PROP_BYPASS_BAND3,
  PROP_BYPASS_BAND4,
  PROP_BYPASS_BAND5,
  PROP_BYPASS_BAND6,
  PROP_BYPASS_BAND7,
  PROP_BYPASS_BAND8,
  PROP_BYPASS_BAND9,
  PROP_BYPASS_BAND10,
  PROP_BYPASS_BAND11,
  PROP_BYPASS_BAND12,
  PROP_RANGE_BEFORE,
  PROP_RANGE_AFTER,
  PROP_AGGRESSIVE,
  PROP_NOTIFY
};

/* pad templates */

static GstStaticPadTemplate gst_pecrystalizer_src_template =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

static GstStaticPadTemplate gst_pecrystalizer_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(GstPecrystalizer,
                        gst_pecrystalizer,
                        GST_TYPE_AUDIO_FILTER,
                        GST_DEBUG_CATEGORY_INIT(gst_pecrystalizer_debug_category,
                                                "pecrystalizer",
                                                0,
                                                "debug category for pecrystalizer element"));

static void gst_pecrystalizer_class_init(GstPecrystalizerClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  GstBaseTransformClass* base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

  GstAudioFilterClass* audio_filter_class = GST_AUDIO_FILTER_CLASS(klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */

  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_pecrystalizer_src_template);
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_pecrystalizer_sink_template);

  gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass), "PulseEffects Crystalizer", "Generic",
                                        "PulseEffects Crystalizer is a port of FFMPEG crystalizer",
                                        "Wellington <wellingtonwallace@gmail.com>");

  /* define virtual function pointers */

  gobject_class->set_property = gst_pecrystalizer_set_property;
  gobject_class->get_property = gst_pecrystalizer_get_property;

  audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_pecrystalizer_setup);

  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR(gst_pecrystalizer_transform_ip);

  base_transform_class->transform_ip_on_passthrough = false;

  base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_pecrystalizer_stop);

  gobject_class->finalize = gst_pecrystalizer_finalize;

  /* define properties */

  for (int n = 0; n < NBANDS; n++) {
    char* name = strdup(std::string("intensity-band" + std::to_string(n)).c_str());
    char* nick = strdup(std::string("BAND " + std::to_string(n) + " INTENSITY").c_str());

    g_object_class_install_property(
        gobject_class, n + 1,
        g_param_spec_float(name, nick, "Expansion intensity", 0.0f, 40.0f, 1.0f,
                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
  }

  for (int n = 0; n < NBANDS; n++) {
    char* name = strdup(std::string("mute-band" + std::to_string(n)).c_str());
    char* nick = strdup(std::string("MUTE BAND " + std::to_string(n)).c_str());

    g_object_class_install_property(
        gobject_class, NBANDS + 1 + n,
        g_param_spec_boolean(name, nick, "mute band", false,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
  }

  for (int n = 0; n < NBANDS; n++) {
    char* name = strdup(std::string("bypass-band" + std::to_string(n)).c_str());
    char* nick = strdup(std::string("BYPASS BAND " + std::to_string(n)).c_str());

    g_object_class_install_property(
        gobject_class, 2 * NBANDS + 1 + n,
        g_param_spec_boolean(name, nick, "bypass band", false,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
  }

  g_object_class_install_property(
      gobject_class, PROP_RANGE_BEFORE,
      g_param_spec_float("lra-before", "Loudness Range", "Loudness Range (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_RANGE_AFTER,
      g_param_spec_float("lra-after", "Loudness Range", "Loudness Range (in LUFS)", -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                         static_cast<GParamFlags>(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_AGGRESSIVE,
      g_param_spec_boolean("aggressive", "Aggressive Mode", "Aggressive Mode", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_NOTIFY,
      g_param_spec_boolean("notify-host", "Notify Host", "Notify host of variable changes", true,
                           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
}

static void gst_pecrystalizer_init(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;
  pecrystalizer->bpf = 0;
  pecrystalizer->nsamples = 0;

  pecrystalizer->freqs[0] = 500.0f;
  pecrystalizer->freqs[1] = 1000.0f;
  pecrystalizer->freqs[2] = 2000.0f;
  pecrystalizer->freqs[3] = 3000.0f;
  pecrystalizer->freqs[4] = 4000.0f;
  pecrystalizer->freqs[5] = 5000.0f;
  pecrystalizer->freqs[6] = 6000.0f;
  pecrystalizer->freqs[7] = 7000.0f;
  pecrystalizer->freqs[8] = 8000.0f;
  pecrystalizer->freqs[9] = 9000.0f;
  pecrystalizer->freqs[10] = 10000.0f;
  pecrystalizer->freqs[11] = 15000.0f;

  for (uint n = 0; n < NBANDS; n++) {
    pecrystalizer->filters[n] = new Filter("crystalizer band" + std::to_string(n));

    pecrystalizer->intensities[n] = 1.0f;
    pecrystalizer->mute[n] = false;
    pecrystalizer->bypass[n] = false;
    pecrystalizer->last_L[n] = 0.0f;
    pecrystalizer->last_R[n] = 0.0f;
  }

  pecrystalizer->sample_count = 0;
  pecrystalizer->notify = false;
  pecrystalizer->range_before = 0.0f;
  pecrystalizer->range_after = 0.0f;
  pecrystalizer->ebur_state_before = nullptr;
  pecrystalizer->ebur_state_after = nullptr;

  pecrystalizer->ndivs = 1000;
  pecrystalizer->dv = 1.0f / pecrystalizer->ndivs;
  pecrystalizer->aggressive = false;

  pecrystalizer->sinkpad = gst_element_get_static_pad(GST_ELEMENT(pecrystalizer), "sink");

  pecrystalizer->srcpad = gst_element_get_static_pad(GST_ELEMENT(pecrystalizer), "src");

  gst_pad_set_query_function(pecrystalizer->srcpad, gst_pecrystalizer_src_query);

  gst_base_transform_set_in_place(GST_BASE_TRANSFORM(pecrystalizer), true);
}

void gst_pecrystalizer_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(object);

  GST_DEBUG_OBJECT(pecrystalizer, "set_property");

  switch (property_id) {
    case PROP_INTENSITY_BAND0:
      pecrystalizer->intensities[0] = g_value_get_float(value);
      pecrystalizer->gain[0] = util::linspace(1.0f, pecrystalizer->intensities[0], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND1:
      pecrystalizer->intensities[1] = g_value_get_float(value);
      pecrystalizer->gain[1] = util::linspace(1.0f, pecrystalizer->intensities[1], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND2:
      pecrystalizer->intensities[2] = g_value_get_float(value);
      pecrystalizer->gain[2] = util::linspace(1.0f, pecrystalizer->intensities[2], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND3:
      pecrystalizer->intensities[3] = g_value_get_float(value);
      pecrystalizer->gain[3] = util::linspace(1.0f, pecrystalizer->intensities[3], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND4:
      pecrystalizer->intensities[4] = g_value_get_float(value);
      pecrystalizer->gain[4] = util::linspace(1.0f, pecrystalizer->intensities[4], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND5:
      pecrystalizer->intensities[5] = g_value_get_float(value);
      pecrystalizer->gain[5] = util::linspace(1.0f, pecrystalizer->intensities[5], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND6:
      pecrystalizer->intensities[6] = g_value_get_float(value);
      pecrystalizer->gain[6] = util::linspace(1.0f, pecrystalizer->intensities[6], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND7:
      pecrystalizer->intensities[7] = g_value_get_float(value);
      pecrystalizer->gain[7] = util::linspace(1.0f, pecrystalizer->intensities[7], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND8:
      pecrystalizer->intensities[8] = g_value_get_float(value);
      pecrystalizer->gain[8] = util::linspace(1.0f, pecrystalizer->intensities[8], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND9:
      pecrystalizer->intensities[9] = g_value_get_float(value);
      pecrystalizer->gain[9] = util::linspace(1.0f, pecrystalizer->intensities[9], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND10:
      pecrystalizer->intensities[10] = g_value_get_float(value);
      pecrystalizer->gain[10] = util::linspace(1.0f, pecrystalizer->intensities[10], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND11:
      pecrystalizer->intensities[11] = g_value_get_float(value);
      pecrystalizer->gain[11] = util::linspace(1.0f, pecrystalizer->intensities[11], pecrystalizer->ndivs);
      break;
    case PROP_INTENSITY_BAND12:
      pecrystalizer->intensities[12] = g_value_get_float(value);
      pecrystalizer->gain[12] = util::linspace(1.0f, pecrystalizer->intensities[12], pecrystalizer->ndivs);
      break;
    case PROP_MUTE_BAND0:
      pecrystalizer->mute[0] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND1:
      pecrystalizer->mute[1] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND2:
      pecrystalizer->mute[2] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND3:
      pecrystalizer->mute[3] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND4:
      pecrystalizer->mute[4] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND5:
      pecrystalizer->mute[5] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND6:
      pecrystalizer->mute[6] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND7:
      pecrystalizer->mute[7] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND8:
      pecrystalizer->mute[8] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND9:
      pecrystalizer->mute[9] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND10:
      pecrystalizer->mute[10] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND11:
      pecrystalizer->mute[11] = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND12:
      pecrystalizer->mute[12] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND0:
      pecrystalizer->bypass[0] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND1:
      pecrystalizer->bypass[1] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND2:
      pecrystalizer->bypass[2] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND3:
      pecrystalizer->bypass[3] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND4:
      pecrystalizer->bypass[4] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND5:
      pecrystalizer->bypass[5] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND6:
      pecrystalizer->bypass[6] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND7:
      pecrystalizer->bypass[7] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND8:
      pecrystalizer->bypass[8] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND9:
      pecrystalizer->bypass[9] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND10:
      pecrystalizer->bypass[10] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND11:
      pecrystalizer->bypass[11] = g_value_get_boolean(value);
      break;
    case PROP_BYPASS_BAND12:
      pecrystalizer->bypass[12] = g_value_get_boolean(value);
      break;
    case PROP_AGGRESSIVE:
      pecrystalizer->aggressive = g_value_get_boolean(value);
      break;
    case PROP_NOTIFY:
      pecrystalizer->notify = g_value_get_boolean(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

void gst_pecrystalizer_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(object);

  GST_DEBUG_OBJECT(pecrystalizer, "get_property");

  switch (property_id) {
    case PROP_INTENSITY_BAND0:
      g_value_set_float(value, pecrystalizer->intensities[0]);
      break;
    case PROP_INTENSITY_BAND1:
      g_value_set_float(value, pecrystalizer->intensities[1]);
      break;
    case PROP_INTENSITY_BAND2:
      g_value_set_float(value, pecrystalizer->intensities[2]);
      break;
    case PROP_INTENSITY_BAND3:
      g_value_set_float(value, pecrystalizer->intensities[3]);
      break;
    case PROP_INTENSITY_BAND4:
      g_value_set_float(value, pecrystalizer->intensities[4]);
      break;
    case PROP_INTENSITY_BAND5:
      g_value_set_float(value, pecrystalizer->intensities[5]);
      break;
    case PROP_INTENSITY_BAND6:
      g_value_set_float(value, pecrystalizer->intensities[6]);
      break;
    case PROP_INTENSITY_BAND7:
      g_value_set_float(value, pecrystalizer->intensities[7]);
      break;
    case PROP_INTENSITY_BAND8:
      g_value_set_float(value, pecrystalizer->intensities[8]);
      break;
    case PROP_INTENSITY_BAND9:
      g_value_set_float(value, pecrystalizer->intensities[9]);
      break;
    case PROP_INTENSITY_BAND10:
      g_value_set_float(value, pecrystalizer->intensities[10]);
      break;
    case PROP_INTENSITY_BAND11:
      g_value_set_float(value, pecrystalizer->intensities[11]);
      break;
    case PROP_INTENSITY_BAND12:
      g_value_set_float(value, pecrystalizer->intensities[12]);
      break;
    case PROP_MUTE_BAND0:
      g_value_set_boolean(value, pecrystalizer->mute[0]);
      break;
    case PROP_MUTE_BAND1:
      g_value_set_boolean(value, pecrystalizer->mute[1]);
      break;
    case PROP_MUTE_BAND2:
      g_value_set_boolean(value, pecrystalizer->mute[2]);
      break;
    case PROP_MUTE_BAND3:
      g_value_set_boolean(value, pecrystalizer->mute[3]);
      break;
    case PROP_MUTE_BAND4:
      g_value_set_boolean(value, pecrystalizer->mute[4]);
      break;
    case PROP_MUTE_BAND5:
      g_value_set_boolean(value, pecrystalizer->mute[5]);
      break;
    case PROP_MUTE_BAND6:
      g_value_set_boolean(value, pecrystalizer->mute[6]);
      break;
    case PROP_MUTE_BAND7:
      g_value_set_boolean(value, pecrystalizer->mute[7]);
      break;
    case PROP_MUTE_BAND8:
      g_value_set_boolean(value, pecrystalizer->mute[8]);
      break;
    case PROP_MUTE_BAND9:
      g_value_set_boolean(value, pecrystalizer->mute[9]);
      break;
    case PROP_MUTE_BAND10:
      g_value_set_boolean(value, pecrystalizer->mute[10]);
      break;
    case PROP_MUTE_BAND11:
      g_value_set_boolean(value, pecrystalizer->mute[11]);
      break;
    case PROP_MUTE_BAND12:
      g_value_set_boolean(value, pecrystalizer->mute[12]);
      break;
    case PROP_BYPASS_BAND0:
      g_value_set_boolean(value, pecrystalizer->bypass[0]);
      break;
    case PROP_BYPASS_BAND1:
      g_value_set_boolean(value, pecrystalizer->bypass[1]);
      break;
    case PROP_BYPASS_BAND2:
      g_value_set_boolean(value, pecrystalizer->bypass[2]);
      break;
    case PROP_BYPASS_BAND3:
      g_value_set_boolean(value, pecrystalizer->bypass[3]);
      break;
    case PROP_BYPASS_BAND4:
      g_value_set_boolean(value, pecrystalizer->bypass[4]);
      break;
    case PROP_BYPASS_BAND5:
      g_value_set_boolean(value, pecrystalizer->bypass[5]);
      break;
    case PROP_BYPASS_BAND6:
      g_value_set_boolean(value, pecrystalizer->bypass[6]);
      break;
    case PROP_BYPASS_BAND7:
      g_value_set_boolean(value, pecrystalizer->bypass[7]);
      break;
    case PROP_BYPASS_BAND8:
      g_value_set_boolean(value, pecrystalizer->bypass[8]);
      break;
    case PROP_BYPASS_BAND9:
      g_value_set_boolean(value, pecrystalizer->bypass[9]);
      break;
    case PROP_BYPASS_BAND10:
      g_value_set_boolean(value, pecrystalizer->bypass[10]);
      break;
    case PROP_BYPASS_BAND11:
      g_value_set_boolean(value, pecrystalizer->bypass[11]);
      break;
    case PROP_BYPASS_BAND12:
      g_value_set_boolean(value, pecrystalizer->bypass[12]);
      break;
    case PROP_RANGE_BEFORE:
      g_value_set_float(value, pecrystalizer->range_before);
      break;
    case PROP_RANGE_AFTER:
      g_value_set_float(value, pecrystalizer->range_after);
      break;
    case PROP_AGGRESSIVE:
      g_value_set_boolean(value, pecrystalizer->aggressive);
      break;
    case PROP_NOTIFY:
      g_value_set_boolean(value, pecrystalizer->notify);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static gboolean gst_pecrystalizer_setup(GstAudioFilter* filter, const GstAudioInfo* info) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(filter);

  GST_DEBUG_OBJECT(pecrystalizer, "setup");

  pecrystalizer->rate = info->rate;
  pecrystalizer->bpf = GST_AUDIO_INFO_BPF(info);

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  gst_pecrystalizer_finish_filters(pecrystalizer);

  /*notify every 0.1 seconds*/

  pecrystalizer->notify_samples = GST_CLOCK_TIME_TO_FRAMES(GST_SECOND / 10, info->rate);

  return true;
}

static GstFlowReturn gst_pecrystalizer_transform_ip(GstBaseTransform* trans, GstBuffer* buffer) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(trans);

  GST_DEBUG_OBJECT(pecrystalizer, "transform");

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READ);

  guint num_samples = map.size / pecrystalizer->bpf;

  gst_buffer_unmap(buffer, &map);

  bool filters_ready = true;

  for (uint n = 0; n < NBANDS; n++) {
    filters_ready = filters_ready && pecrystalizer->filters[n]->ready;
  }

  if (filters_ready) {
    if (pecrystalizer->nsamples == num_samples) {
      gst_pecrystalizer_process(pecrystalizer, buffer);
    } else {
      gst_pecrystalizer_finish_filters(pecrystalizer);
    }
  } else {
    pecrystalizer->nsamples = num_samples;

    gst_pecrystalizer_finish_filters(pecrystalizer);
    gst_pecrystalizer_setup_filters(pecrystalizer);

    gst_element_post_message(GST_ELEMENT_CAST(pecrystalizer), gst_message_new_latency(GST_OBJECT_CAST(pecrystalizer)));
  }

  return GST_FLOW_OK;
}

static gboolean gst_pecrystalizer_stop(GstBaseTransform* base) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(base);

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  gst_pecrystalizer_finish_filters(pecrystalizer);

  return true;
}

static void gst_pecrystalizer_setup_filters(GstPecrystalizer* pecrystalizer) {
  if (pecrystalizer->rate != 0) {
    for (uint n = 0; n < NBANDS; n++) {
      if (pecrystalizer->band_data[n].size() != 2 * pecrystalizer->nsamples) {
        pecrystalizer->band_data[n].resize(2 * pecrystalizer->nsamples);
      }
    }

    if (pecrystalizer->deriv2.size() != 2 * pecrystalizer->nsamples) {
      pecrystalizer->deriv2.resize(2 * pecrystalizer->nsamples);
    }

    /*
      Bandpass transition band has to be twice the value used for lowpass and
      highpass. This way all filters will have the same delay.
    */

    float transition_band = 100.0f;  // Hz

    for (uint n = 0; n < NBANDS; n++) {
      if (n == 0) {
        pecrystalizer->filters[0]->create_lowpass(pecrystalizer->nsamples, pecrystalizer->rate, pecrystalizer->freqs[0],
                                                  transition_band);
      } else if (n == pecrystalizer->filters.size() - 1) {
        pecrystalizer->filters[n]->create_highpass(pecrystalizer->nsamples, pecrystalizer->rate,
                                                   pecrystalizer->freqs.back(), transition_band);
      } else {
        pecrystalizer->filters[n]->create_bandpass(pecrystalizer->nsamples, pecrystalizer->rate,
                                                   pecrystalizer->freqs[n - 1], pecrystalizer->freqs[n],
                                                   2.0f * transition_band);
      }
    }

    // before

    pecrystalizer->ebur_state_before = ebur128_init(2, pecrystalizer->rate, EBUR128_MODE_LRA | EBUR128_MODE_HISTOGRAM);

    ebur128_set_channel(pecrystalizer->ebur_state_before, 0, EBUR128_LEFT);
    ebur128_set_channel(pecrystalizer->ebur_state_before, 1, EBUR128_RIGHT);

    ebur128_set_max_history(pecrystalizer->ebur_state_before, 30 * 1000);  // ms

    // after

    pecrystalizer->ebur_state_after = ebur128_init(2, pecrystalizer->rate, EBUR128_MODE_LRA | EBUR128_MODE_HISTOGRAM);

    ebur128_set_channel(pecrystalizer->ebur_state_after, 0, EBUR128_LEFT);
    ebur128_set_channel(pecrystalizer->ebur_state_after, 1, EBUR128_RIGHT);

    ebur128_set_max_history(pecrystalizer->ebur_state_after, 30 * 1000);  // ms
  }
}

static void gst_pecrystalizer_process(GstPecrystalizer* pecrystalizer, GstBuffer* buffer) {
  bool ebur_failed = false;
  double range;
  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  float* data = (float*)map.data;

  /* Measure loudness range before the processing. Rigorously speaking we should
     add the band_data arrays because we will delay output by 1 sample. But I
     think this sample will not affect the measruing that much.
   */

  if (pecrystalizer->notify) {
    ebur128_add_frames_float(pecrystalizer->ebur_state_before, data, pecrystalizer->nsamples);

    if (EBUR128_SUCCESS != ebur128_loudness_range(pecrystalizer->ebur_state_before, &range)) {
      ebur_failed = true;
    } else {
      pecrystalizer->range_before = (float)range;
    }
  }

  for (uint n = 0; n < NBANDS; n++) {
    memcpy(pecrystalizer->band_data[n].data(), data, map.size);

    pecrystalizer->filters[n]->process(pecrystalizer->band_data[n].data());

    /*
      Later we will need to calculate the second derivative of each band. This
      is done through the central difference method. In order to calculate
      themderivative at the last elementsof the array we have to now the first
      element of the next buffer. As we do not have this information the only
      way to do this calculation is delaying the signal by 1 sample.
    */

    // last (R,L) becomes the first
    std::rotate(pecrystalizer->band_data[n].rbegin(), pecrystalizer->band_data[n].rbegin() + 2,
                pecrystalizer->band_data[n].rend());

    if (!pecrystalizer->ready) {
      /*pecrystalizer->band_data was rotated. Its first values are the
        last ones from the original array. Now we save the last (R,L) values for
        the next round
      */
      pecrystalizer->delayed_L[n] = pecrystalizer->band_data[n][0];
      pecrystalizer->delayed_R[n] = pecrystalizer->band_data[n][1];

      // first elements becomes silence
      pecrystalizer->band_data[n][0] = 0.0f;
      pecrystalizer->band_data[n][1] = 0.0f;

      pecrystalizer->last_L[n] = 0.0f;
      pecrystalizer->last_R[n] = 0.0f;

      if (n == NBANDS - 1) {
        pecrystalizer->ready = true;
      }
    } else {
      /*pecrystalizer->band_data was rotated. Its first values are the
        last ones from the original array. we have to save them for the next
        round.
      */
      float L = pecrystalizer->band_data[n][0];
      float R = pecrystalizer->band_data[n][1];

      /*the previously delayed (R,L) pair becomes the first element of this
        round.
       */
      pecrystalizer->band_data[n][0] = pecrystalizer->delayed_L[n];
      pecrystalizer->band_data[n][1] = pecrystalizer->delayed_R[n];

      // saving the last (R,L) values for the next round
      pecrystalizer->delayed_L[n] = L;
      pecrystalizer->delayed_R[n] = R;
    }
  }

  for (uint n = 0; n < NBANDS; n++) {
    if (!pecrystalizer->bypass[n]) {
      // Calculating second derivative

      for (uint m = 0; m < pecrystalizer->nsamples; m++) {
        float L = pecrystalizer->band_data[n][2 * m];
        float R = pecrystalizer->band_data[n][2 * m + 1];

        if (m > 0 && m < pecrystalizer->nsamples - 1) {
          float L_lower = pecrystalizer->band_data[n][2 * (m - 1)];
          float R_lower = pecrystalizer->band_data[n][2 * (m - 1) + 1];
          float L_upper = pecrystalizer->band_data[n][2 * (m + 1)];
          float R_upper = pecrystalizer->band_data[n][2 * (m + 1) + 1];

          pecrystalizer->deriv2[2 * m] = L_upper - 2 * L + L_lower;
          pecrystalizer->deriv2[2 * m + 1] = R_upper - 2 * R + R_lower;
        } else if (m == 0) {
          float L_upper = pecrystalizer->band_data[n][2 * (m + 1)];
          float R_upper = pecrystalizer->band_data[n][2 * (m + 1) + 1];

          pecrystalizer->deriv2[2 * m] = L_upper - 2 * L + pecrystalizer->last_L[n];
          pecrystalizer->deriv2[2 * m + 1] = R_upper - 2 * R + pecrystalizer->last_R[n];
        } else if (m == pecrystalizer->nsamples - 1) {
          float L_upper = pecrystalizer->delayed_L[n];
          float R_upper = pecrystalizer->delayed_R[n];
          float L_lower = pecrystalizer->band_data[n][2 * (m - 1)];
          float R_lower = pecrystalizer->band_data[n][2 * (m - 1) + 1];

          pecrystalizer->deriv2[2 * m] = L_upper - 2 * L + L_lower;
          pecrystalizer->deriv2[2 * m + 1] = R_upper - 2 * R + R_lower;
        }
      }

      // peak enhancing using second derivative

      for (uint m = 0; m < pecrystalizer->nsamples; m++) {
        float L = pecrystalizer->band_data[n][2 * m];
        float R = pecrystalizer->band_data[n][2 * m + 1];
        float d2L = pecrystalizer->deriv2[2 * m];
        float d2R = pecrystalizer->deriv2[2 * m + 1];

        pecrystalizer->band_data[n][2 * m] = L - pecrystalizer->intensities[n] * d2L;
        pecrystalizer->band_data[n][2 * m + 1] = R - pecrystalizer->intensities[n] * d2R;

        /*
          Aggressive mode applies a amplitude dependent gain to every sample in
          the signal
        */

        if (pecrystalizer->aggressive && pecrystalizer->gain[n].size() != 0) {
          uint idx_L = floorf(fabsf(L) / pecrystalizer->dv);
          uint idx_R = floorf(fabsf(R) / pecrystalizer->dv);

          if (idx_L < 0) {
            idx_L = 0;
          } else if (idx_L > pecrystalizer->gain[n].size()) {
            idx_L = pecrystalizer->gain[n].size() - 1;
          }

          if (idx_R < 0) {
            idx_R = 0;
          } else if (idx_R > pecrystalizer->gain[n].size()) {
            idx_R = pecrystalizer->gain[n].size() - 1;
          }

          float vL = pecrystalizer->band_data[n][2 * m];
          float vR = pecrystalizer->band_data[n][2 * m + 1];

          pecrystalizer->band_data[n][2 * m] = vL * pecrystalizer->gain[n][idx_L];

          pecrystalizer->band_data[n][2 * m + 1] = vR * pecrystalizer->gain[n][idx_R];
        }

        if (m == pecrystalizer->nsamples - 1) {
          pecrystalizer->last_L[n] = L;
          pecrystalizer->last_R[n] = R;
        }
      }
    } else {
      pecrystalizer->last_L[n] = pecrystalizer->band_data[n][2 * pecrystalizer->nsamples - 2];
      pecrystalizer->last_R[n] = pecrystalizer->band_data[n][2 * pecrystalizer->nsamples - 1];
    }
  }

  // add bands

  for (uint n = 0; n < 2 * pecrystalizer->nsamples; n++) {
    data[n] = 0.0f;

    for (uint m = 0; m < pecrystalizer->filters.size(); m++) {
      if (!pecrystalizer->mute[m]) {
        data[n] += pecrystalizer->band_data[m][n];
      }
    }
  }

  // Measure loudness range after the processing

  if (pecrystalizer->notify) {
    ebur128_add_frames_float(pecrystalizer->ebur_state_after, data, pecrystalizer->nsamples);

    if (EBUR128_SUCCESS != ebur128_loudness_range(pecrystalizer->ebur_state_after, &range)) {
      ebur_failed = true;
    } else {
      pecrystalizer->range_after = (float)range;
    }
  }

  gst_buffer_unmap(buffer, &map);

  if (!ebur_failed && pecrystalizer->notify) {
    pecrystalizer->sample_count += pecrystalizer->nsamples;

    if (pecrystalizer->sample_count >= pecrystalizer->notify_samples) {
      pecrystalizer->sample_count = 0;

      // std::cout << "range: " << pecrystalizer->range_before << "\t"
      //           << pecrystalizer->range_after << std::endl;

      g_object_notify(G_OBJECT(pecrystalizer), "lra-before");
      g_object_notify(G_OBJECT(pecrystalizer), "lra-after");
    }
  }
}

static gboolean gst_pecrystalizer_src_query(GstPad* pad, GstObject* parent, GstQuery* query) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(parent);
  bool ret = true;

  switch (GST_QUERY_TYPE(query)) {
    case GST_QUERY_LATENCY:
      if (pecrystalizer->rate > 0) {
        ret = gst_pad_peer_query(pecrystalizer->sinkpad, query);

        if (ret) {
          GstClockTime min, max;
          gboolean live;
          guint64 latency;

          gst_query_parse_latency(query, &live, &min, &max);

          /* add our own latency */

          latency = gst_util_uint64_scale_round(1, GST_SECOND, pecrystalizer->rate);

          // std::cout << "latency: " << latency << std::endl;
          // std::cout << "n: " << pecrystalizer->inbuf_n_samples
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
    default:
      /* just call the default handler */
      ret = gst_pad_query_default(pad, parent, query);
      break;
  }

  return ret;
}

static void gst_pecrystalizer_finish_filters(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;

  for (uint m = 0; m < NBANDS; m++) {
    pecrystalizer->filters[m]->finish();
  }

  if (pecrystalizer->ebur_state_before != nullptr) {
    ebur128_destroy(&pecrystalizer->ebur_state_before);
    pecrystalizer->ebur_state_before = nullptr;
  }

  if (pecrystalizer->ebur_state_after != nullptr) {
    ebur128_destroy(&pecrystalizer->ebur_state_after);
    pecrystalizer->ebur_state_after = nullptr;
  }
}

void gst_pecrystalizer_finalize(GObject* object) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(object);

  GST_DEBUG_OBJECT(pecrystalizer, "finalize");

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  gst_pecrystalizer_finish_filters(pecrystalizer);

  /* clean up object here */

  G_OBJECT_CLASS(gst_pecrystalizer_parent_class)->finalize(object);
}

static gboolean plugin_init(GstPlugin* plugin) {
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "pecrystalizer", GST_RANK_NONE, GST_TYPE_PECRYSTALIZER);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  pecrystalizer,
                  "PulseEffects Crystalizer",
                  plugin_init,
                  VERSION,
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
