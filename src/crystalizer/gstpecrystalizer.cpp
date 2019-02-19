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

static void gst_pecrystalizer_setup_filters(GstPecrystalizer* pecrystalizer);

static void gst_pecrystalizer_process(GstPecrystalizer* pecrystalizer,
                                      GstBuffer* buffer);

static void gst_pecrystalizer_finish_filters(GstPecrystalizer* pecrystalizer);

enum {
  PROP_0,
  PROP_INTENSITY_BAND0,
  PROP_INTENSITY_BAND1,
  PROP_INTENSITY_BAND2,
  PROP_INTENSITY_BAND3,
  PROP_MUTE_BAND0,
  PROP_MUTE_BAND1,
  PROP_MUTE_BAND2,
  PROP_MUTE_BAND3,
  PROP_FREQ1,
  PROP_FREQ2,
  PROP_FREQ3
};

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
      gobject_class, PROP_FREQ1,
      g_param_spec_float(
          "freq1", "SPLIT FREQUENCY 1",
          "Split frequency between the first and the second band", 10.0f,
          20000.0f, 500.0f,
          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                   G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_FREQ2,
      g_param_spec_float(
          "freq2", "SPLIT FREQUENCY 2",
          "Split frequency between the second and the third band", 10.0f,
          20000.0f, 3000.0f,
          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                   G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_FREQ3,
      g_param_spec_float(
          "freq3", "SPLIT FREQUENCY 3",
          "Split frequency between the third and the fourth band", 10.0f,
          20000.0f, 6000.0f,
          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                   G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_BAND0,
      g_param_spec_float("intensity-band0", "BAND 0 INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 4.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_BAND1,
      g_param_spec_float("intensity-band1", "BAND 1 INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 2.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_BAND2,
      g_param_spec_float("intensity-band2", "BAND 2 INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 1.0f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_INTENSITY_BAND3,
      g_param_spec_float("intensity-band3", "BAND 3 INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 0.5f,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_MUTE_BAND0,
      g_param_spec_boolean("mute-band0", "MUTE BAND 0", "mute band", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_MUTE_BAND1,
      g_param_spec_boolean("mute-band1", "MUTE BAND 1", "mute band", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_MUTE_BAND2,
      g_param_spec_boolean("mute-band2", "MUTE BAND 2", "mute band", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_MUTE_BAND3,
      g_param_spec_boolean("mute-band3", "MUTE BAND 3", "mute band", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));
}

static void gst_pecrystalizer_init(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;
  pecrystalizer->bpf = 0;
  pecrystalizer->nsamples = 0;

  pecrystalizer->freq1 = 500.0f;
  pecrystalizer->freq2 = 3000.0f;
  pecrystalizer->freq3 = 6000.0f;
  pecrystalizer->intensity_band0 = 4.0f;
  pecrystalizer->intensity_band1 = 2.0f;
  pecrystalizer->intensity_band2 = 1.0f;
  pecrystalizer->intensity_band3 = 0.5f;
  pecrystalizer->mute_band0 = false;
  pecrystalizer->mute_band1 = false;
  pecrystalizer->mute_band2 = false;
  pecrystalizer->mute_band3 = false;

  pecrystalizer->last_L_band0 = 0.0f;
  pecrystalizer->last_L_band1 = 0.0f;
  pecrystalizer->last_L_band2 = 0.0f;
  pecrystalizer->last_L_band3 = 0.0f;
  pecrystalizer->last_R_band0 = 0.0f;
  pecrystalizer->last_R_band1 = 0.0f;
  pecrystalizer->last_R_band2 = 0.0f;
  pecrystalizer->last_R_band3 = 0.0f;

  pecrystalizer->band0_lowpass = new Filter(Mode::lowpass, "band0");

  pecrystalizer->band1_lowpass = new Filter(Mode::lowpass, "band1");
  pecrystalizer->band1_highpass = new Filter(Mode::highpass, "band1");

  pecrystalizer->band2_lowpass = new Filter(Mode::lowpass, "band2");
  pecrystalizer->band2_highpass = new Filter(Mode::highpass, "band2");

  pecrystalizer->band3_highpass = new Filter(Mode::highpass, "band3");

  gst_base_transform_set_in_place(GST_BASE_TRANSFORM(pecrystalizer), true);
}

void gst_pecrystalizer_set_property(GObject* object,
                                    guint property_id,
                                    const GValue* value,
                                    GParamSpec* pspec) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(object);

  GST_DEBUG_OBJECT(pecrystalizer, "set_property");

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  switch (property_id) {
    case PROP_FREQ1:
      pecrystalizer->freq1 = g_value_get_float(value);

      gst_pecrystalizer_finish_filters(pecrystalizer);

      break;
    case PROP_FREQ2:
      pecrystalizer->freq2 = g_value_get_float(value);

      gst_pecrystalizer_finish_filters(pecrystalizer);

      break;
    case PROP_FREQ3:
      pecrystalizer->freq3 = g_value_get_float(value);

      gst_pecrystalizer_finish_filters(pecrystalizer);

      break;
    case PROP_INTENSITY_BAND0:
      pecrystalizer->intensity_band0 = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND1:
      pecrystalizer->intensity_band1 = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND2:
      pecrystalizer->intensity_band2 = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND3:
      pecrystalizer->intensity_band3 = g_value_get_float(value);
      break;
    case PROP_MUTE_BAND0:
      pecrystalizer->mute_band0 = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND1:
      pecrystalizer->mute_band1 = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND2:
      pecrystalizer->mute_band2 = g_value_get_boolean(value);
      break;
    case PROP_MUTE_BAND3:
      pecrystalizer->mute_band3 = g_value_get_boolean(value);
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
    case PROP_FREQ1:
      g_value_set_float(value, pecrystalizer->freq1);
      break;
    case PROP_FREQ2:
      g_value_set_float(value, pecrystalizer->freq2);
      break;
    case PROP_FREQ3:
      g_value_set_float(value, pecrystalizer->freq3);
      break;
    case PROP_INTENSITY_BAND0:
      g_value_set_float(value, pecrystalizer->intensity_band0);
      break;
    case PROP_INTENSITY_BAND1:
      g_value_set_float(value, pecrystalizer->intensity_band1);
      break;
    case PROP_INTENSITY_BAND2:
      g_value_set_float(value, pecrystalizer->intensity_band2);
      break;
    case PROP_INTENSITY_BAND3:
      g_value_set_float(value, pecrystalizer->intensity_band3);
      break;
    case PROP_MUTE_BAND0:
      g_value_set_float(value, pecrystalizer->mute_band0);
      break;
    case PROP_MUTE_BAND1:
      g_value_set_float(value, pecrystalizer->mute_band1);
      break;
    case PROP_MUTE_BAND2:
      g_value_set_float(value, pecrystalizer->mute_band2);
      break;
    case PROP_MUTE_BAND3:
      g_value_set_float(value, pecrystalizer->mute_band3);
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

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  gst_pecrystalizer_finish_filters(pecrystalizer);

  return true;
}

static GstFlowReturn gst_pecrystalizer_transform_ip(GstBaseTransform* trans,
                                                    GstBuffer* buffer) {
  GstPecrystalizer* pecrystalizer = GST_PECRYSTALIZER(trans);

  GST_DEBUG_OBJECT(pecrystalizer, "transform");

  std::lock_guard<std::mutex> lock(pecrystalizer->mutex);

  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READ);

  guint num_samples = map.size / pecrystalizer->bpf;

  gst_buffer_unmap(buffer, &map);

  if (pecrystalizer->band0_lowpass->ready &&
      pecrystalizer->band1_lowpass->ready &&
      pecrystalizer->band1_highpass->ready &&
      pecrystalizer->band2_lowpass->ready &&
      pecrystalizer->band2_highpass->ready &&
      pecrystalizer->band3_highpass->ready) {
    if (pecrystalizer->nsamples == num_samples) {
      gst_pecrystalizer_process(pecrystalizer, buffer);
    } else {
      gst_pecrystalizer_finish_filters(pecrystalizer);
    }
  } else {
    pecrystalizer->nsamples = num_samples;

    gst_pecrystalizer_setup_filters(pecrystalizer);
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
    pecrystalizer->data_band0 = new float[2 * pecrystalizer->nsamples];
    pecrystalizer->data_band1 = new float[2 * pecrystalizer->nsamples];
    pecrystalizer->data_band2 = new float[2 * pecrystalizer->nsamples];
    pecrystalizer->data_band3 = new float[2 * pecrystalizer->nsamples];

    float transition_band = 50.0f;

    // band 0

    pecrystalizer->band0_lowpass->init_kernel(
        pecrystalizer->rate, pecrystalizer->freq1, transition_band);
    pecrystalizer->band0_lowpass->init_zita(pecrystalizer->nsamples);

    // band 1

    pecrystalizer->band1_lowpass->init_kernel(
        pecrystalizer->rate, pecrystalizer->freq2, transition_band);
    pecrystalizer->band1_lowpass->init_zita(pecrystalizer->nsamples);

    pecrystalizer->band1_highpass->init_kernel(
        pecrystalizer->rate, pecrystalizer->freq1, transition_band);
    pecrystalizer->band1_highpass->init_zita(pecrystalizer->nsamples);

    // band 2

    pecrystalizer->band2_lowpass->init_kernel(
        pecrystalizer->rate, pecrystalizer->freq3, transition_band);
    pecrystalizer->band2_lowpass->init_zita(pecrystalizer->nsamples);

    pecrystalizer->band2_highpass->init_kernel(
        pecrystalizer->rate, pecrystalizer->freq2, transition_band);
    pecrystalizer->band2_highpass->init_zita(pecrystalizer->nsamples);

    // band 3

    pecrystalizer->band3_highpass->init_kernel(
        pecrystalizer->rate, pecrystalizer->freq3, transition_band);
    pecrystalizer->band3_highpass->init_zita(pecrystalizer->nsamples);
  }
}

static void gst_pecrystalizer_process(GstPecrystalizer* pecrystalizer,
                                      GstBuffer* buffer) {
  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  float* data = (float*)map.data;

  memcpy(pecrystalizer->data_band0, data, map.size);
  memcpy(pecrystalizer->data_band1, data, map.size);
  memcpy(pecrystalizer->data_band2, data, map.size);
  memcpy(pecrystalizer->data_band3, data, map.size);

  pecrystalizer->band0_lowpass->process(pecrystalizer->data_band0);

  // band 1 bandpass
  pecrystalizer->band1_lowpass->process(pecrystalizer->data_band1);
  pecrystalizer->band1_highpass->process(pecrystalizer->data_band1);

  // band 2 bandpass
  pecrystalizer->band2_lowpass->process(pecrystalizer->data_band2);
  pecrystalizer->band2_highpass->process(pecrystalizer->data_band2);

  pecrystalizer->band3_highpass->process(pecrystalizer->data_band3);

  if (!pecrystalizer->ready) {
    pecrystalizer->last_L_band0 = pecrystalizer->data_band0[0];
    pecrystalizer->last_R_band0 = pecrystalizer->data_band0[1];

    pecrystalizer->last_L_band1 = pecrystalizer->data_band1[0];
    pecrystalizer->last_R_band1 = pecrystalizer->data_band1[1];

    pecrystalizer->last_L_band2 = pecrystalizer->data_band2[0];
    pecrystalizer->last_R_band2 = pecrystalizer->data_band2[1];

    pecrystalizer->last_L_band3 = pecrystalizer->data_band3[0];
    pecrystalizer->last_R_band3 = pecrystalizer->data_band3[1];

    pecrystalizer->ready = true;
  }

  /*Code taken from FFMPEG crystalizer plugin
   *https://git.ffmpeg.org/gitweb/ffmpeg.git/blob_plain/HEAD:/libavfilter/af_crystalizer.c
   */

  auto process_sample = [&](float*& band_data, uint& n, float& intensity,
                            float& last_L, float& last_R, bool& mute) {
    if (!mute) {
      float L = band_data[2 * n];
      float R = band_data[2 * n + 1];

      band_data[2 * n] = L + (L - last_L) * intensity;
      band_data[2 * n + 1] = R + (R - last_R) * intensity;

      last_L = L;
      last_R = R;
    } else if (n == pecrystalizer->nsamples - 1) {
      last_L = band_data[2 * n];
      last_R = band_data[2 * n + 1];
    }
  };

  for (uint n = 0; n < pecrystalizer->nsamples; n++) {
    // band 0

    process_sample(pecrystalizer->data_band0, n, pecrystalizer->intensity_band0,
                   pecrystalizer->last_L_band0, pecrystalizer->last_R_band0,
                   pecrystalizer->mute_band0);

    // band 1

    process_sample(pecrystalizer->data_band1, n, pecrystalizer->intensity_band1,
                   pecrystalizer->last_L_band1, pecrystalizer->last_R_band1,
                   pecrystalizer->mute_band1);

    // band 2

    process_sample(pecrystalizer->data_band2, n, pecrystalizer->intensity_band2,
                   pecrystalizer->last_L_band2, pecrystalizer->last_R_band2,
                   pecrystalizer->mute_band2);

    // band 3

    process_sample(pecrystalizer->data_band3, n, pecrystalizer->intensity_band3,
                   pecrystalizer->last_L_band3, pecrystalizer->last_R_band3,
                   pecrystalizer->mute_band3);
  }

  // add bands

  for (unsigned int n = 0; n < 2 * pecrystalizer->nsamples; n++) {
    data[n] = pecrystalizer->data_band0[n] + pecrystalizer->data_band1[n] +
              pecrystalizer->data_band2[n] + pecrystalizer->data_band3[n];
  }

  gst_buffer_unmap(buffer, &map);
}

static void gst_pecrystalizer_finish_filters(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;

  pecrystalizer->band0_lowpass->finish();

  pecrystalizer->band1_lowpass->finish();
  pecrystalizer->band1_highpass->finish();

  pecrystalizer->band2_lowpass->finish();
  pecrystalizer->band2_highpass->finish();

  pecrystalizer->band3_highpass->finish();

  auto free_data = [](float*& data) {
    if (data != nullptr) {
      delete[] data;

      data = nullptr;
    }
  };

  free_data(pecrystalizer->data_band0);
  free_data(pecrystalizer->data_band1);
  free_data(pecrystalizer->data_band2);
  free_data(pecrystalizer->data_band3);
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
