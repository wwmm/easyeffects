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

#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <cmath>
#include <iostream>
#include "gstpeautogain.hpp"

GST_DEBUG_CATEGORY_STATIC(gst_peautogain_debug_category);
#define GST_CAT_DEFAULT gst_peautogain_debug_category

/* prototypes */

static void gst_peautogain_set_property(GObject* object,
                                        guint property_id,
                                        const GValue* value,
                                        GParamSpec* pspec);

static void gst_peautogain_get_property(GObject* object,
                                        guint property_id,
                                        GValue* value,
                                        GParamSpec* pspec);

static gboolean gst_peautogain_setup(GstAudioFilter* filter,
                                     const GstAudioInfo* info);

static GstFlowReturn gst_peautogain_transform_ip(GstBaseTransform* trans,
                                                 GstBuffer* buffer);

static gboolean gst_peautogain_stop(GstBaseTransform* base);

static void gst_peautogain_finalize(GObject* object);

static void gst_peautogain_process(GstPeautogain* peautogain,
                                   GstBuffer* buffer);

enum { PROP_0, PROP_WINDOW, PROP_TARGET };

/* pad templates */

static GstStaticPadTemplate gst_peautogain_src_template =
    GST_STATIC_PAD_TEMPLATE(
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

static GstStaticPadTemplate gst_peautogain_sink_template =
    GST_STATIC_PAD_TEMPLATE(
        "sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstPeautogain,
    gst_peautogain,
    GST_TYPE_AUDIO_FILTER,
    GST_DEBUG_CATEGORY_INIT(gst_peautogain_debug_category,
                            "peautogain",
                            0,
                            "debug category for peautogain element"));

static void gst_peautogain_class_init(GstPeautogainClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    GstBaseTransformClass* base_transform_class =
        GST_BASE_TRANSFORM_CLASS(klass);

    GstAudioFilterClass* audio_filter_class = GST_AUDIO_FILTER_CLASS(klass);

    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */

    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peautogain_src_template);
    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peautogain_sink_template);

    gst_element_class_set_static_metadata(
        GST_ELEMENT_CLASS(klass), "PulseEffects ebur128 level meter", "Generic",
        "PulseEffects ebur128 level meter",
        "Wellington <wellingtonwallace@gmail.com>");

    /* define virtual function pointers */

    gobject_class->set_property = gst_peautogain_set_property;
    gobject_class->get_property = gst_peautogain_get_property;

    audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_peautogain_setup);

    base_transform_class->transform_ip =
        GST_DEBUG_FUNCPTR(gst_peautogain_transform_ip);

    base_transform_class->transform_ip_on_passthrough = false;

    base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_peautogain_stop);

    gobject_class->finalize = gst_peautogain_finalize;

    /* define properties */

    g_object_class_install_property(
        gobject_class, PROP_WINDOW,
        g_param_spec_int("window", "Window", "ebur128 window (in milliseconds)",
                         1, 3000, 400,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_TARGET,
        g_param_spec_float("target", "Target Level",
                           "Target loudness level (in LUFS)", -100.0f, 0.0f,
                           -23.0f,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS)));
}

static void gst_peautogain_init(GstPeautogain* peautogain) {
    peautogain->ready = false;
    peautogain->bpf = 0;
    peautogain->rate = 0;
    peautogain->window = 400;     // ms
    peautogain->target = -23.0f;  // LUFS
    peautogain->gain = 1.0f;

    gst_base_transform_set_in_place(GST_BASE_TRANSFORM(peautogain), true);
}

void gst_peautogain_set_property(GObject* object,
                                 guint property_id,
                                 const GValue* value,
                                 GParamSpec* pspec) {
    GstPeautogain* peautogain = GST_PEAUTOGAIN(object);

    GST_DEBUG_OBJECT(peautogain, "set_property");

    switch (property_id) {
        case PROP_WINDOW:
            peautogain->window = g_value_get_int(value);
            break;
        case PROP_TARGET:
            peautogain->target = g_value_get_float(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_peautogain_get_property(GObject* object,
                                 guint property_id,
                                 GValue* value,
                                 GParamSpec* pspec) {
    GstPeautogain* peautogain = GST_PEAUTOGAIN(object);

    GST_DEBUG_OBJECT(peautogain, "get_property");

    switch (property_id) {
        case PROP_WINDOW:
            g_value_set_int(value, peautogain->window);
            break;
        case PROP_TARGET:
            g_value_set_float(value, peautogain->target);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static gboolean gst_peautogain_setup(GstAudioFilter* filter,
                                     const GstAudioInfo* info) {
    GstPeautogain* peautogain = GST_PEAUTOGAIN(filter);

    GST_DEBUG_OBJECT(peautogain, "setup");

    peautogain->bpf = info->bpf;
    peautogain->rate = info->rate;

    return true;
}

static GstFlowReturn gst_peautogain_transform_ip(GstBaseTransform* trans,
                                                 GstBuffer* buffer) {
    GstPeautogain* peautogain = GST_PEAUTOGAIN(trans);

    GST_DEBUG_OBJECT(peautogain, "transform");

    std::lock_guard<std::mutex> lock(peautogain->lock_guard_ebu);

    if (peautogain->ready) {
        gst_peautogain_process(peautogain, buffer);
    } else {
        peautogain->ebur_state = ebur128_init(
            2, peautogain->rate,
            EBUR128_MODE_HISTOGRAM | EBUR128_MODE_I | EBUR128_MODE_SAMPLE_PEAK);

        ebur128_set_channel(peautogain->ebur_state, 0, EBUR128_LEFT);
        ebur128_set_channel(peautogain->ebur_state, 1, EBUR128_RIGHT);

        ebur128_set_max_window(peautogain->ebur_state, 3000);  // ms

        peautogain->ready = true;
    }

    return GST_FLOW_OK;
}

static gboolean gst_peautogain_stop(GstBaseTransform* base) {
    GstPeautogain* peautogain = GST_PEAUTOGAIN(base);

    std::lock_guard<std::mutex> lock(peautogain->lock_guard_ebu);

    peautogain->ready = false;
    peautogain->gain = 1.0f;

    if (peautogain->ebur_state) {
        ebur128_destroy(&peautogain->ebur_state);
        free(peautogain->ebur_state);
    }

    return true;
}

void gst_peautogain_finalize(GObject* object) {
    GstPeautogain* peautogain = GST_PEAUTOGAIN(object);

    GST_DEBUG_OBJECT(peautogain, "finalize");

    std::lock_guard<std::mutex> lock(peautogain->lock_guard_ebu);

    peautogain->ready = false;
    peautogain->gain = 1.0f;

    if (peautogain->ebur_state) {
        ebur128_destroy(&peautogain->ebur_state);
        free(peautogain->ebur_state);
    }

    G_OBJECT_CLASS(gst_peautogain_parent_class)->finalize(object);
}

static void gst_peautogain_process(GstPeautogain* peautogain,
                                   GstBuffer* buffer) {
    GstMapInfo map;

    gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

    float* data = (float*)map.data;

    guint num_samples = map.size / peautogain->bpf;

    ebur128_add_frames_float(peautogain->ebur_state, data, num_samples);

    double relative, loudness;
    bool failed = false;

    if (EBUR128_SUCCESS != ebur128_loudness_window(peautogain->ebur_state,
                                                   peautogain->window,
                                                   &loudness)) {
        loudness = 0.0;
        failed = true;
    }

    if (EBUR128_SUCCESS !=
        ebur128_relative_threshold(peautogain->ebur_state, &relative)) {
        relative = 0.0;
        failed = true;
    }

    if (loudness > relative && relative > -70 && !failed) {
        double peak, peak_L, peak_R;

        if (EBUR128_SUCCESS !=
            ebur128_prev_sample_peak(peautogain->ebur_state, 0, &peak_L)) {
            peak_L = 0.0;
            failed = true;
        }

        if (EBUR128_SUCCESS !=
            ebur128_prev_sample_peak(peautogain->ebur_state, 1, &peak_R)) {
            peak_R = 0.0;
            failed = true;
        }

        if (!failed) {
            peak = (peak_L > peak_R) ? peak_L : peak_R;

            float diff = peautogain->target - (float)loudness;

            // 10^(diff/20). The way below should be faster than using pow
            float gain = expf((diff / 20.0f) * logf(10.0f));

            if (gain * peak < 1) {
                peautogain->gain = gain;
            } else {
                peautogain->gain = fabsf(1.0f / (float)peak);
            }

            // std::cout << "relative: " << relative << std::endl;
            // std::cout << "loudness: " << loudness << std::endl;
            // std::cout << "gain: " << peautogain->gain << std::endl;
        }
    }

    for (unsigned int n = 0; n < 2 * num_samples; n++) {
        data[n] = data[n] * peautogain->gain;
    }

    gst_buffer_unmap(buffer, &map);
}

static gboolean plugin_init(GstPlugin* plugin) {
    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register(plugin, "peautogain", GST_RANK_NONE,
                                GST_TYPE_PEAUTOGAIN);
}

#ifndef PACKAGE
#define PACKAGE "PulseEffects"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  peautogain,
                  "PulseEffects autogain based on libebur128",
                  plugin_init,
                  "4.2.1",
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
