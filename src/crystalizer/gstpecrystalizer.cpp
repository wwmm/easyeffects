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
  PROP_BYPASS_BAND12
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

  for (int n = 0; n < NBANDS; n++) {
    char* name =
        strdup(std::string("intensity-band" + std::to_string(n)).c_str());
    char* nick =
        strdup(std::string("BAND " + std::to_string(n) + " INTENSITY").c_str());

    g_object_class_install_property(
        gobject_class, n + 1,
        g_param_spec_float(name, nick, "Expansion intensity", 0.0f, 40.0f, 1.0f,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));
  }

  for (int n = 0; n < NBANDS; n++) {
    char* name = strdup(std::string("mute-band" + std::to_string(n)).c_str());
    char* nick = strdup(std::string("MUTE BAND " + std::to_string(n)).c_str());

    g_object_class_install_property(
        gobject_class, NBANDS + 1 + n,
        g_param_spec_boolean(name, nick, "mute band", false,
                             static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS)));
  }

  for (int n = 0; n < NBANDS; n++) {
    char* name = strdup(std::string("bypass-band" + std::to_string(n)).c_str());
    char* nick =
        strdup(std::string("BYPASS BAND " + std::to_string(n)).c_str());

    g_object_class_install_property(
        gobject_class, 2 * NBANDS + 1 + n,
        g_param_spec_boolean(name, nick, "bypass band", false,
                             static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS)));
  }
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
    pecrystalizer->filters[n] =
        new Filter("crystalizer band" + std::to_string(n));

    pecrystalizer->intensities[n] = 1.0f;
    pecrystalizer->mute[n] = false;
    pecrystalizer->bypass[n] = false;
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
    case PROP_INTENSITY_BAND5:
      pecrystalizer->intensities[5] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND6:
      pecrystalizer->intensities[6] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND7:
      pecrystalizer->intensities[7] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND8:
      pecrystalizer->intensities[8] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND9:
      pecrystalizer->intensities[9] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND10:
      pecrystalizer->intensities[10] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND11:
      pecrystalizer->intensities[11] = g_value_get_float(value);
      break;
    case PROP_INTENSITY_BAND12:
      pecrystalizer->intensities[12] = g_value_get_float(value);
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
    for (uint n = 0; n < NBANDS; n++) {
      pecrystalizer->band_data[n].resize(2 * pecrystalizer->nsamples);
    }

    pecrystalizer->tmp_band_data.resize(2 * pecrystalizer->nsamples);

    /*
      Bandpass transition band has to be twice the value used for lowpass and
      highpass. This way all filters will have the same delay.
    */

    float transition_band = 50.0f;  // Hz

    for (uint n = 0; n < NBANDS; n++) {
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

  for (uint n = 0; n < NBANDS; n++) {
    memcpy(pecrystalizer->band_data[n].data(), data, map.size);

    pecrystalizer->filters[n]->process(pecrystalizer->band_data[n].data());
  }

  if (!pecrystalizer->ready) {
    for (uint n = 0; n < NBANDS; n++) {
      pecrystalizer->last_L[n] = pecrystalizer->band_data[n][0];
      pecrystalizer->last_R[n] = pecrystalizer->band_data[n][1];
    }

    pecrystalizer->ready = true;
  }

  /*This algorithm is based on the one from FFMPEG crystalizer plugin
   *https://git.ffmpeg.org/gitweb/ffmpeg.git/blob_plain/HEAD:/libavfilter/af_crystalizer.c
   */

  for (uint n = 0; n < NBANDS; n++) {
    if (!pecrystalizer->bypass[n]) {
      memcpy(pecrystalizer->tmp_band_data.data(),
             pecrystalizer->band_data[n].data(), map.size);

      for (uint m = 0; m < pecrystalizer->nsamples; m++) {
        float L = pecrystalizer->band_data[n][2 * m];
        float R = pecrystalizer->band_data[n][2 * m + 1];

        // ffmpeg algorithm

        float v1_L =
            L + (L - pecrystalizer->last_L[n]) * pecrystalizer->intensities[n];
        float v1_R =
            R + (R - pecrystalizer->last_R[n]) * pecrystalizer->intensities[n];

        /*
         This modification avoids time shifts in the signal and a few
         undesirable distortions in the waveform. See the graph made by
         /util/crystalizer.py
        */

        float v2_L = 0.0f, v2_R = 0.0f;

        if (m < pecrystalizer->nsamples - 1) {
          float L_upper = pecrystalizer->band_data[n][2 * (m + 1)];
          float R_upper = pecrystalizer->band_data[n][2 * (m + 1) + 1];

          v2_L = L + (L - L_upper) * pecrystalizer->intensities[n];
          v2_R = R + (R - R_upper) * pecrystalizer->intensities[n];
        }

        pecrystalizer->band_data[n][2 * m] = 0.5f * (v1_L + v2_L);
        pecrystalizer->band_data[n][2 * m + 1] = 0.5f * (v1_R + v2_R);

        pecrystalizer->last_L[n] = L;
        pecrystalizer->last_R[n] = R;
      }

      // applying ffmpeg algorithm from buffer end to start

      // auto last_L =
      //     pecrystalizer->tmp_band_data[2 * pecrystalizer->nsamples - 2];
      //
      // auto last_R =
      //     pecrystalizer->tmp_band_data[2 * pecrystalizer->nsamples - 1];
      //
      // for (int m = pecrystalizer->nsamples - 1; m >= 0; m--) {
      //   float L = pecrystalizer->tmp_band_data[2 * m];
      //   float R = pecrystalizer->tmp_band_data[2 * m + 1];
      //
      //   pecrystalizer->tmp_band_data[2 * m] =
      //       L + (L - last_L) * pecrystalizer->intensities[n];
      //
      //   pecrystalizer->tmp_band_data[2 * m + 1] =
      //       R + (R - last_R) * pecrystalizer->intensities[n];
      //
      //   last_L = L;
      //   last_R = R;
      // }

      /// taking the average of both runs

      // for (uint m = 0; m < 2 * pecrystalizer->nsamples; m++) {
      //   pecrystalizer->band_data[n][m] += pecrystalizer->tmp_band_data[m];
      //
      //   pecrystalizer->band_data[n][m] *= 0.5f;
      // }
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
      if (!pecrystalizer->mute[m]) {
        data[n] += pecrystalizer->band_data[m][n];
      }
    }
  }

  gst_buffer_unmap(buffer, &map);
}

static void gst_pecrystalizer_finish_filters(GstPecrystalizer* pecrystalizer) {
  pecrystalizer->ready = false;

  for (uint m = 0; m < NBANDS; m++) {
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
