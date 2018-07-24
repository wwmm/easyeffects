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

static void gst_peebur_recalc_interval_frames(GstPeebur* peebur);

enum { PROP_0, PROP_POST_MESSAGES, PROP_INTERVAL };

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
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static gboolean gst_peebur_setup(GstAudioFilter* filter,
                                 const GstAudioInfo* info) {
    GstPeebur* peebur = GST_PEEBUR(filter);

    GST_DEBUG_OBJECT(peebur, "setup");

    peebur->bpf = info->bpf;
    peebur->rate = info->rate;
    peebur->ebur_state = ebur128_init(2, info->rate, EBUR128_MODE_I);

    gst_peebur_recalc_interval_frames(peebur);

    std::cout << peebur->bpf << std::endl;

    return true;
}

static GstFlowReturn gst_peebur_transform_ip(GstBaseTransform* trans,
                                             GstBuffer* buffer) {
    GstPeebur* peebur = GST_PEEBUR(trans);

    GST_DEBUG_OBJECT(peebur, "transform");

    gst_buffer_ref(buffer);
    gst_adapter_push(peebur->adapter, buffer);

    gsize nbytes = peebur->interval_frames * peebur->bpf;

    while (gst_adapter_available(peebur->adapter) >= nbytes) {
        const float* data = (float*)gst_adapter_map(peebur->adapter, nbytes);

        ebur128_add_frames_float(peebur->ebur_state, data,
                                 peebur->interval_frames);

        ebur128_loudness_global(peebur->ebur_state, &peebur->loudness);

        std::cout << peebur->loudness << std::endl;

        gst_adapter_unmap(peebur->adapter);
        gst_adapter_flush(peebur->adapter, nbytes);
    }

    return GST_FLOW_OK;
}

static gboolean gst_peebur_stop(GstBaseTransform* base) {
    GstPeebur* peebur = GST_PEEBUR(base);

    peebur->ready = false;

    ebur128_destroy(&peebur->ebur_state);
    free(peebur->ebur_state);

    gst_adapter_clear(peebur->adapter);
    g_object_unref(peebur->adapter);

    return true;
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
