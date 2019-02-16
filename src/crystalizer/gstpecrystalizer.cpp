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
#include "config.h"

GST_DEBUG_CATEGORY_STATIC(gst_pecrystalizer_debug_category);
#define GST_CAT_DEFAULT gst_pecrystalizer_debug_category

/* prototypes */

static void gst_pecrystalizer_set_property(GObject* object,
                                           guint property_id,
                                           const GValue* value,
                                           GParamSpec* pspec);

static void gst_pecrystalizer_get_property(GObject* object,
                                           guint property_id,
                                           GValue* value,
                                           GParamSpec* pspec);

static gboolean gst_pecrystalizer_setup(GstAudioFilter* filter,
                                        const GstAudioInfo* info);

static GstFlowReturn gst_pecrystalizer_transform_ip(GstBaseTransform* trans,
                                                    GstBuffer* buffer);

static gboolean gst_pecrystalizer_stop(GstBaseTransform* base);

enum { PROP_0, PROP_INTENSITY_LOW, PROP_INTENSITY_MID, PROP_INTENSITY_HIGH };

/* pad templates */

static GstStaticPadTemplate gst_pecrystalizer_src_template =
    GST_STATIC_PAD_TEMPLATE(
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

static GstStaticPadTemplate gst_pecrystalizer_sink_template =
    GST_STATIC_PAD_TEMPLATE(
        "sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstPecrystalizer,
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

  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                            &gst_pecrystalizer_src_template);
  gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                            &gst_pecrystalizer_sink_template);

  gst_element_class_set_static_metadata(
      GST_ELEMENT_CLASS(klass), "PulseEffects Crystalizer", "Generic",
      "PulseEffects Crystalizer is a port of FFMPEG crystalizer",
      "Wellington <wellingtonwallace@gmail.com>");

  /* define virtual function pointers */

  gobject_class->set_property = gst_pecrystalizer_set_property;
  gobject_class->get_property = gst_pecrystalizer_get_property;

  audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_pecrystalizer_setup);

  base_transform_class->transform_ip =
      GST_DEBUG_FUNCPTR(gst_pecrystalizer_transform_ip);

  base_transform_class->transform_ip_on_passthrough = false;

  base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_pecrystalizer_stop);

  /* define properties */

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_LOW,
      g_param_spec_float("intensity-low", "LOW BAND INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 2.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_MID,
      g_param_spec_float("intensity-mid", "MID BAND INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 2.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_HIGH,
      g_param_spec_float("intensity-high", "HIGH BAND INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 2.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));
}

static void gst_pecrystalizer_init(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;
  pecrystalizer->bpf = 0;
  pecrystalizer->intensity_low = 2.0f;
  pecrystalizer->intensity_mid = 1.0f;
  pecrystalizer->intensity_high = 0.5f;
  pecrystalizer->last_L = 0.0f;
  pecrystalizer->last_R = 0.0f;

  pecrystalizer->lowpass = new Filter(Mode::lowpass, 3000, 100);
  pecrystalizer->highpass = new Filter(Mode::highpass, 10000, 100);

  pecrystalizer->bandlow = new Filter(Mode::lowpass, 10000, 100);
  pecrystalizer->bandhigh = new Filter(Mode::highpass, 3000, 100);

  gst_base_transform_set_in_place(GST_BASE_TRANSFORM(pecrystalizer), true);
}

void gst_pecrystalizer_set_property(GObject* object,
                                    guint property_id,
                                    const GValue* value,
                                    GParamSpec* pspec) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(object);

  GST_DEBUG_OBJECT(pecrystalizer, "set_property");

  switch (property_id) {
    case PROP_INTENSITY_LOW:
      pecrystalizer->intensity_low = g_value_get_float(value);
      break;
    case PROP_INTENSITY_MID:
      pecrystalizer->intensity_mid = g_value_get_float(value);
      break;
    case PROP_INTENSITY_HIGH:
      pecrystalizer->intensity_high = g_value_get_float(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

void gst_pecrystalizer_get_property(GObject* object,
                                    guint property_id,
                                    GValue* value,
                                    GParamSpec* pspec) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(object);

  GST_DEBUG_OBJECT(pecrystalizer, "get_property");

  switch (property_id) {
    case PROP_INTENSITY_LOW:
      g_value_set_float(value, pecrystalizer->intensity_low);
      break;
    case PROP_INTENSITY_MID:
      g_value_set_float(value, pecrystalizer->intensity_mid);
      break;
    case PROP_INTENSITY_HIGH:
      g_value_set_float(value, pecrystalizer->intensity_high);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static gboolean gst_pecrystalizer_setup(GstAudioFilter* filter,
                                        const GstAudioInfo* info) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(filter);

  GST_DEBUG_OBJECT(pecrystalizer, "setup");

  pecrystalizer->rate = info->rate;
  pecrystalizer->bpf = GST_AUDIO_INFO_BPF(info);

  return true;
}

static GstFlowReturn gst_pecrystalizer_transform_ip(GstBaseTransform* trans,
                                                    GstBuffer* buffer) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(trans);

  GST_DEBUG_OBJECT(pecrystalizer, "transform");

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  guint num_samples = map.size / pecrystalizer->bpf;

  float* data = (float*)map.data;

  if (pecrystalizer->lowpass->ready && pecrystalizer->highpass->ready) {
    memcpy(pecrystalizer->data_low, data, map.size);
    memcpy(pecrystalizer->data_high, data, map.size);

    pecrystalizer->lowpass->process(pecrystalizer->data_low);
    pecrystalizer->highpass->process(pecrystalizer->data_high);

    pecrystalizer->bandlow->process(data);
    pecrystalizer->bandhigh->process(data);
  } else {
    if (pecrystalizer->data_low != nullptr) {
      delete[] pecrystalizer->data_low;

      pecrystalizer->data_low = nullptr;
    }
    if (pecrystalizer->data_high != nullptr) {
      delete[] pecrystalizer->data_high;

      pecrystalizer->data_high = nullptr;
    }

    pecrystalizer->data_low = new float[2 * num_samples];
    pecrystalizer->data_high = new float[2 * num_samples];

    pecrystalizer->lowpass->init_kernel(pecrystalizer->rate);
    pecrystalizer->lowpass->init_zita(num_samples);

    pecrystalizer->highpass->init_kernel(pecrystalizer->rate);
    pecrystalizer->highpass->init_zita(num_samples);

    pecrystalizer->bandlow->init_kernel(pecrystalizer->rate);
    pecrystalizer->bandlow->init_zita(num_samples);

    pecrystalizer->bandhigh->init_kernel(pecrystalizer->rate);
    pecrystalizer->bandhigh->init_zita(num_samples);
  }

  if (!pecrystalizer->ready) {
    pecrystalizer->last_L = data[0];
    pecrystalizer->last_R = data[1];
    pecrystalizer->ready = true;
  }

  /*Code taken from FFMPEG crystalizer plugin
   *https://git.ffmpeg.org/gitweb/ffmpeg.git/blob_plain/HEAD:/libavfilter/af_crystalizer.c
   */

  // for (unsigned int n = 0; n < num_samples; n++) {
  //   float L = data[2 * n], R = data[2 * n + 1];
  //
  //   data[2 * n] = L + (L - pecrystalizer->last_L) * pecrystalizer->intensity;
  //
  //   data[2 * n + 1] =
  //       R + (R - pecrystalizer->last_R) * pecrystalizer->intensity;
  //
  //   pecrystalizer->last_L = L;
  //   pecrystalizer->last_R = R;
  // }

  gst_buffer_unmap(buffer, &map);

  return GST_FLOW_OK;
}

static gboolean gst_pecrystalizer_stop(GstBaseTransform* base) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(base);

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  pecrystalizer->ready = false;

  return true;
}

static gboolean plugin_init(GstPlugin* plugin) {
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "pecrystalizer", GST_RANK_NONE,
                              GST_TYPE_PECRYSTALIZER);
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
