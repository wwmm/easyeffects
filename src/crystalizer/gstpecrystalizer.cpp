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
  PROP_INTENSITY_BAND4,
  PROP_MUTE_BAND0,
  PROP_MUTE_BAND1,
  PROP_MUTE_BAND2,
  PROP_MUTE_BAND3,
  PROP_MUTE_BAND4,
  PROP_FREQ1,
  PROP_FREQ2,
  PROP_FREQ3,
  PROP_FREQ4
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
          20000.0f, 1250.0f,
          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                   G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_FREQ2,
      g_param_spec_float(
          "freq2", "SPLIT FREQUENCY 2",
          "Split frequency between the second and the third band", 10.0f,
          20000.0f, 2500.0f,
          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                   G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_FREQ3,
      g_param_spec_float(
          "freq3", "SPLIT FREQUENCY 3",
          "Split frequency between the third and the fourth band", 10.0f,
          20000.0f, 5000.0f,
          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                   G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property(
      gobject_class, PROP_FREQ4,
      g_param_spec_float(
          "freq4", "SPLIT FREQUENCY 4",
          "Split frequency between the fourth and the fifth band", 10.0f,
          20000.0f, 10000.0f,
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
      gobject_class, PROP_INTENSITY_BAND4,
      g_param_spec_float("intensity-band4", "BAND 4 INTENSITY",
                         "Expansion intensity", 0.0f, 10.0f, 0.25f,
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

  g_object_class_install_property(
      gobject_class, PROP_MUTE_BAND4,
      g_param_spec_boolean("mute-band4", "MUTE BAND 4", "mute band", false,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));
}

static void gst_pecrystalizer_init(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;
  pecrystalizer->bpf = 0;
  pecrystalizer->nsamples = 0;

  pecrystalizer->freq1 = 1250.0f;
  pecrystalizer->freq2 = 2500.0f;
  pecrystalizer->freq3 = 5000.0f;
  pecrystalizer->freq4 = 10000.0f;

  pecrystalizer->freqs[0] = 1250.0f;
  pecrystalizer->freqs[1] = 2500.0f;
  pecrystalizer->freqs[2] = 5000.0f;
  pecrystalizer->freqs[3] = 10000.0f;

  for (uint n = 0; n < pecrystalizer->filters.size(); n++) {
    pecrystalizer->filters[n] =
        new Filter("crystalizer band" + std::to_string(n));

    pecrystalizer->intensities[n] = 1.0f;
    pecrystalizer->mute[n] = false;
    pecrystalizer->last_L[n] = 0.0f;
    pecrystalizer->last_R[n] = 0.0f;
  }

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
    case PROP_FREQ4:
      pecrystalizer->freq4 = g_value_get_float(value);

      gst_pecrystalizer_finish_filters(pecrystalizer);

      break;
    case PROP_INTENSITY_BAND0:
      pecrystalizer->intensities[0] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND1:
      pecrystalizer->intensities[1] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND2:
      pecrystalizer->intensities[2] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND3:
      pecrystalizer->intensities[3] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND4:
      pecrystalizer->intensities[4] = g_value_get_float(value);
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
    case PROP_FREQ4:
      g_value_set_float(value, pecrystalizer->freq4);
      break;
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
    case PROP_MUTE_BAND0:
      g_value_set_float(value, pecrystalizer->mute[0]);
      break;
    case PROP_MUTE_BAND1:
      g_value_set_float(value, pecrystalizer->mute[1]);
      break;
    case PROP_MUTE_BAND2:
      g_value_set_float(value, pecrystalizer->mute[2]);
      break;
    case PROP_MUTE_BAND3:
      g_value_set_float(value, pecrystalizer->mute[3]);
      break;
    case PROP_MUTE_BAND4:
      g_value_set_float(value, pecrystalizer->mute[4]);
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

  bool filters_ready = true;

  for (uint n = 0; n < pecrystalizer->filters.size(); n++) {
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

    auto f = [=]() {
      std::lock_guard<std::mutex> lock(pecrystalizer->mutex);
      gst_pecrystalizer_setup_filters(pecrystalizer);
    };

    pecrystalizer->futures.clear();

    auto future = std::async(std::launch::async, f);

    pecrystalizer->futures.push_back(std::move(future));
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
    for (uint n = 0; n < pecrystalizer->band_data.size(); n++) {
      pecrystalizer->band_data[n].resize(2 * pecrystalizer->nsamples);
    }

    /*
      Bandpass transition band has to be twice the value used for lowpass and
      highpass. This way all filters will have the same delay.
    */

    float transition_band = 50.0f;  // Hz

    for (uint n = 0; n < pecrystalizer->filters.size(); n++) {
      if (n == 0) {
        pecrystalizer->filters[0]->create_lowpass(
            pecrystalizer->nsamples, pecrystalizer->rate,
            pecrystalizer->freqs[0], transition_band);
      } else if (n == pecrystalizer->filters.size() - 1) {
        pecrystalizer->filters[n]->create_highpass(
            pecrystalizer->nsamples, pecrystalizer->rate,
            pecrystalizer->freqs.back(), transition_band);
      } else {
        pecrystalizer->filters[n]->create_bandpass(
            pecrystalizer->nsamples, pecrystalizer->rate,
            pecrystalizer->freqs[n - 1], pecrystalizer->freqs[n],
            2.0f * transition_band);
      }
    }
  }
}

static void gst_pecrystalizer_process(GstPecrystalizer* pecrystalizer,
                                      GstBuffer* buffer) {
  GstMapInfo map;

  gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

  float* data = (float*)map.data;

  for (uint n = 0; n < pecrystalizer->band_data.size(); n++) {
    memcpy(pecrystalizer->band_data[n].data(), data, map.size);

    pecrystalizer->filters[n]->process(pecrystalizer->band_data[n].data());
  }

  if (!pecrystalizer->ready) {
    for (uint n = 0; n < pecrystalizer->band_data.size(); n++) {
      pecrystalizer->last_L[n] = pecrystalizer->band_data[n][0];
      pecrystalizer->last_R[n] = pecrystalizer->band_data[n][1];
    }

    pecrystalizer->ready = true;
  }

  /*Code adapted from FFMPEG crystalizer plugin
   *https://git.ffmpeg.org/gitweb/ffmpeg.git/blob_plain/HEAD:/libavfilter/af_crystalizer.c
   */

  for (uint n = 0; n < pecrystalizer->filters.size(); n++) {
    if (!pecrystalizer->mute[n]) {
      for (uint m = 0; m < pecrystalizer->nsamples; m++) {
        float L = pecrystalizer->band_data[n][2 * m];
        float R = pecrystalizer->band_data[n][2 * m + 1];

        pecrystalizer->band_data[n][2 * m] =
            L + (L - pecrystalizer->last_L[n]) * pecrystalizer->intensities[n];

        pecrystalizer->band_data[n][2 * m + 1] =
            R + (R - pecrystalizer->last_R[n]) * pecrystalizer->intensities[n];

        pecrystalizer->last_L[n] = L;
        pecrystalizer->last_R[n] = R;
      }
    } else {
      pecrystalizer->last_L[n] =
          pecrystalizer->band_data[n][2 * pecrystalizer->nsamples - 2];
      pecrystalizer->last_R[n] =
          pecrystalizer->band_data[n][2 * pecrystalizer->nsamples - 1];
    }
  }

  // add bands

  for (unsigned int n = 0; n < 2 * pecrystalizer->nsamples; n++) {
    data[n] = 0.0f;

    for (uint m = 0; m < pecrystalizer->filters.size(); m++) {
      data[n] += pecrystalizer->band_data[m][n];
    }
  }

  gst_buffer_unmap(buffer, &map);
}

static void gst_pecrystalizer_finish_filters(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;

  for (uint m = 0; m < pecrystalizer->filters.size(); m++) {
    pecrystalizer->filters[m]->finish();
  }

  pecrystalizer->futures.clear();
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
