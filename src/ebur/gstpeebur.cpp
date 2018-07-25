/**
 * SECTION:element-gstpeebur
 *
 * The peebur element is a level meter based on libebur128.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v audiotestsrc ! peebur ! pulsesink
 * ]|
 * The peebur element is a level meter based on libebur128.
 * </refsect2>
 */

#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <cmath>
#include <iostream>
#include "gstpeebur.hpp"

GST_DEBUG_CATEGORY_STATIC(gst_peebur_debug_category);
#define GST_CAT_DEFAULT gst_peebur_debug_category

/* prototypes */

static void gst_peebur_set_property(GObject* object,
                                    guint property_id,
                                    const GValue* value,
                                    GParamSpec* pspec);

static void gst_peebur_get_property(GObject* object,
                                    guint property_id,
                                    GValue* value,
                                    GParamSpec* pspec);

static gboolean gst_peebur_setup(GstAudioFilter* filter,
                                 const GstAudioInfo* info);

static GstFlowReturn gst_peebur_transform_ip(GstBaseTransform* trans,
                                             GstBuffer* buffer);

static gboolean gst_peebur_stop(GstBaseTransform* base);

static void gst_peebur_finalize(GObject* object);

static void gst_peebur_recalc_interval_frames(GstPeebur* peebur);

enum {
    PROP_0,
    PROP_POST_MESSAGES,
    PROP_INTERVAL,
    PROP_LOUDNESS,
    PROP_MAX_PEAK
};

/* pad templates */

static GstStaticPadTemplate gst_peebur_src_template = GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                    "channels=2,layout=interleaved"));

static GstStaticPadTemplate gst_peebur_sink_template = GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                    "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstPeebur,
    gst_peebur,
    GST_TYPE_AUDIO_FILTER,
    GST_DEBUG_CATEGORY_INIT(gst_peebur_debug_category,
                            "peebur",
                            0,
                            "debug category for peebur element"));

static void gst_peebur_class_init(GstPeeburClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    GstBaseTransformClass* base_transform_class =
        GST_BASE_TRANSFORM_CLASS(klass);

    GstAudioFilterClass* audio_filter_class = GST_AUDIO_FILTER_CLASS(klass);

    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */

    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peebur_src_template);
    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peebur_sink_template);

    gst_element_class_set_static_metadata(
        GST_ELEMENT_CLASS(klass), "PulseEffects ebur128 level meter", "Generic",
        "PulseEffects ebur128 level meter",
        "Wellington <wellingtonwallace@gmail.com>");

    /* define virtual function pointers */

    gobject_class->set_property = gst_peebur_set_property;
    gobject_class->get_property = gst_peebur_get_property;

    audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_peebur_setup);

    base_transform_class->transform_ip =
        GST_DEBUG_FUNCPTR(gst_peebur_transform_ip);

    base_transform_class->transform_ip_on_passthrough = false;

    base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_peebur_stop);

    gobject_class->finalize = gst_peebur_finalize;

    /* define properties */

    g_object_class_install_property(
        gobject_class, PROP_POST_MESSAGES,
        g_param_spec_boolean(
            "post-messages", "Post Messages",
            "Whether to post a 'level' element message on the bus for each "
            "passed interval",
            true,
            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                     G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_INTERVAL,
        g_param_spec_uint64(
            "interval", "Interval",
            "Interval of time between message posts (in nanoseconds)", 1,
            G_MAXUINT64, GST_SECOND / 10,
            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                     G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_LOUDNESS,
        g_param_spec_double("loudness", "Loudness", "Measured loudness",
                            -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                            static_cast<GParamFlags>(G_PARAM_READABLE |
                                                     G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_MAX_PEAK,
        g_param_spec_double("max-peak", "Maximum Peak",
                            "Maximum Peak Between the channels", -G_MAXDOUBLE,
                            G_MAXDOUBLE, 0,
                            static_cast<GParamFlags>(G_PARAM_READABLE |
                                                     G_PARAM_STATIC_STRINGS)));
}

static void gst_peebur_init(GstPeebur* peebur) {
    peebur->ready = false;
    peebur->bpf = 0;
    peebur->rate = 0;
    peebur->post_messages = true;
    peebur->interval = GST_SECOND / 10;
    peebur->adapter = gst_adapter_new();

    gst_base_transform_set_in_place(GST_BASE_TRANSFORM(peebur), true);
}

void gst_peebur_set_property(GObject* object,
                             guint property_id,
                             const GValue* value,
                             GParamSpec* pspec) {
    GstPeebur* peebur = GST_PEEBUR(object);

    GST_DEBUG_OBJECT(peebur, "set_property");

    switch (property_id) {
        case PROP_POST_MESSAGES:
            peebur->post_messages = g_value_get_boolean(value);

            if (!peebur->post_messages) {
                gst_adapter_clear(peebur->adapter);
            }

            break;
        case PROP_INTERVAL:
            peebur->interval = g_value_get_uint64(value);

            if (peebur->rate > 0) {
                gst_peebur_recalc_interval_frames(peebur);
            }

            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_peebur_get_property(GObject* object,
                             guint property_id,
                             GValue* value,
                             GParamSpec* pspec) {
    GstPeebur* peebur = GST_PEEBUR(object);

    GST_DEBUG_OBJECT(peebur, "get_property");

    switch (property_id) {
        case PROP_POST_MESSAGES:
            g_value_set_boolean(value, peebur->post_messages);
            break;
        case PROP_INTERVAL:
            g_value_set_uint64(value, peebur->interval);
            break;
        case PROP_LOUDNESS:
            g_value_set_double(value, peebur->loudness);
            break;
        case PROP_MAX_PEAK:
            g_value_set_double(value, peebur->max_peak);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static gboolean gst_peebur_setup(GstAudioFilter* filter,
                                 const GstAudioInfo* info) {
    GstPeebur* peebur = GST_PEEBUR(filter);

    GST_DEBUG_OBJECT(peebur, "setup");

    std::lock_guard<std::mutex> lock(peebur->lock_guard_ebu);

    peebur->bpf = info->bpf;
    peebur->rate = info->rate;
    peebur->ebur_state = ebur128_init(
        2, info->rate,
        EBUR128_MODE_HISTOGRAM | EBUR128_MODE_S | EBUR128_MODE_SAMPLE_PEAK);

    ebur128_set_channel(peebur->ebur_state, 0, EBUR128_LEFT);
    ebur128_set_channel(peebur->ebur_state, 1, EBUR128_RIGHT);

    gst_peebur_recalc_interval_frames(peebur);

    ebur128_set_max_window(peebur->ebur_state, 10000);  // ms

    peebur->ready = true;

    return true;
}

static GstFlowReturn gst_peebur_transform_ip(GstBaseTransform* trans,
                                             GstBuffer* buffer) {
    GstPeebur* peebur = GST_PEEBUR(trans);

    GST_DEBUG_OBJECT(peebur, "transform");

    std::lock_guard<std::mutex> lock(peebur->lock_guard_ebu);

    if (peebur->post_messages && peebur->ready) {
        gst_buffer_ref(buffer);
        gst_adapter_push(peebur->adapter, buffer);

        gsize nbytes = peebur->interval_frames * peebur->bpf;

        while (gst_adapter_available(peebur->adapter) >= nbytes) {
            double peak_L, peak_R;

            const float* data =
                (float*)gst_adapter_map(peebur->adapter, nbytes);

            ebur128_add_frames_float(peebur->ebur_state, data,
                                     peebur->interval_frames);

            ebur128_loudness_shortterm(peebur->ebur_state, &peebur->loudness);
            ebur128_prev_sample_peak(peebur->ebur_state, 0, &peak_L);
            ebur128_prev_sample_peak(peebur->ebur_state, 1, &peak_R);

            peak_L = 20 * log10(peak_L);
            peak_R = 20 * log10(peak_R);

            peebur->max_peak = (peak_L > peak_R) ? peak_L : peak_R;

            // std::cout << "left: " << peak_L << "\t"
            //           << "right :" << peak_R << std::endl;

            gst_adapter_unmap(peebur->adapter);
            gst_adapter_flush(peebur->adapter, nbytes);

            g_object_notify(G_OBJECT(peebur), "loudness");
            g_object_notify(G_OBJECT(peebur), "max-peak");
        }
    }

    return GST_FLOW_OK;
}

static gboolean gst_peebur_stop(GstBaseTransform* base) {
    // GstPeebur* peebur = GST_PEEBUR(base);

    return true;
}

void gst_peebur_finalize(GObject* object) {
    GstPeebur* peebur = GST_PEEBUR(object);

    GST_DEBUG_OBJECT(peebur, "finalize");

    std::lock_guard<std::mutex> lock(peebur->lock_guard_ebu);

    peebur->ready = false;

    // ebur128_destroy(&peebur->ebur_state);
    // free(peebur->ebur_state);

    gst_adapter_clear(peebur->adapter);
    g_object_unref(peebur->adapter);

    G_OBJECT_CLASS(gst_peebur_parent_class)->finalize(object);
}

static void gst_peebur_recalc_interval_frames(GstPeebur* peebur) {
    GstClockTime interval = peebur->interval;
    guint interval_frames;

    interval_frames = GST_CLOCK_TIME_TO_FRAMES(interval, peebur->rate);

    if (interval_frames == 0) {
        interval_frames = 1;
    }

    peebur->interval_frames = interval_frames;
}

static gboolean plugin_init(GstPlugin* plugin) {
    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register(plugin, "peebur", GST_RANK_NONE,
                                GST_TYPE_PEEBUR);
}

#ifndef PACKAGE
#define PACKAGE "PulseEffects"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  peebur,
                  "PulseEffects ebur128 level meter",
                  plugin_init,
                  "4.2.1",
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
