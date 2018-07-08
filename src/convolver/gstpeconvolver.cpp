/**
 * SECTION:element-gstpeconvolver
 *
 * The peconvolver element does convolution with inpulse responses.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v audiotestsrc ! peconvolver ! pulsesink
 * ]|
 * The peconvolver element does convolution with inpulse responses.
 * </refsect2>
 */

#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <iostream>
#include "gstpeconvolver.hpp"
#include "read_kernel.hpp"

GST_DEBUG_CATEGORY_STATIC(gst_peconvolver_debug_category);
#define GST_CAT_DEFAULT gst_peconvolver_debug_category

/* prototypes */

static void gst_peconvolver_set_property(GObject* object,
                                         guint property_id,
                                         const GValue* value,
                                         GParamSpec* pspec);

static void gst_peconvolver_get_property(GObject* object,
                                         guint property_id,
                                         GValue* value,
                                         GParamSpec* pspec);

static void gst_peconvolver_finalize(GObject* object);

static gboolean gst_peconvolver_setup(GstAudioFilter* filter,
                                      const GstAudioInfo* info);

static GstFlowReturn gst_peconvolver_transform(GstBaseTransform* trans,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf);

static GstStateChangeReturn gst_peconvolver_change_state(
    GstElement* element,
    GstStateChange transition);

static void process(GstPeconvolver* peconvolver, GstBuffer* buffer);

static void setup_convolver(GstPeconvolver* peconvolver);

static void finish_convolver(GstPeconvolver* peconvolver);

/*global variables*/

enum { PROP_0, PROP_KERNEL_PATH };

/* pad templates */

/* FIXME add/remove the formats that you want to support */
static GstStaticPadTemplate gst_peconvolver_src_template =
    GST_STATIC_PAD_TEMPLATE(
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

/* FIXME add/remove the formats that you want to support */
static GstStaticPadTemplate gst_peconvolver_sink_template =
    GST_STATIC_PAD_TEMPLATE(
        "sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(
    GstPeconvolver,
    gst_peconvolver,
    GST_TYPE_AUDIO_FILTER,
    GST_DEBUG_CATEGORY_INIT(gst_peconvolver_debug_category,
                            "peconvolver",
                            0,
                            "debug category for peconvolver element"));

static void gst_peconvolver_class_init(GstPeconvolverClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    GstBaseTransformClass* base_transform_class =
        GST_BASE_TRANSFORM_CLASS(klass);

    GstAudioFilterClass* audio_filter_class = GST_AUDIO_FILTER_CLASS(klass);

    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);

    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */

    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peconvolver_src_template);
    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peconvolver_sink_template);

    gst_element_class_set_static_metadata(
        GST_ELEMENT_CLASS(klass), "PulseEffects Convolver", "Generic",
        "PulseEffects Convolver", "Wellington <wellingtonwallace@gmail.com>");

    /* define virtual function pointers */

    gobject_class->set_property = gst_peconvolver_set_property;
    gobject_class->get_property = gst_peconvolver_get_property;

    gobject_class->finalize = gst_peconvolver_finalize;

    audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_peconvolver_setup);

    base_transform_class->transform =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform);

    element_class->change_state = gst_peconvolver_change_state;

    /* define properties */

    g_object_class_install_property(
        gobject_class, PROP_KERNEL_PATH,
        g_param_spec_string("kernelpath", "Kernel Path",
                            "Full path to kernel file", nullptr,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS)));
}

static void gst_peconvolver_init(GstPeconvolver* peconvolver) {
    peconvolver->conv_buffer_size = 1024;
}

void gst_peconvolver_set_property(GObject* object,
                                  guint property_id,
                                  const GValue* value,
                                  GParamSpec* pspec) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "set_property");

    switch (property_id) {
        case PROP_KERNEL_PATH:
            peconvolver->kernel_path = g_value_dup_string(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_peconvolver_get_property(GObject* object,
                                  guint property_id,
                                  GValue* value,
                                  GParamSpec* pspec) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "get_property");

    switch (property_id) {
        case PROP_KERNEL_PATH:
            g_value_set_string(value, peconvolver->kernel_path);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_peconvolver_finalize(GObject* object) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "finalize");

    finish_convolver(peconvolver);

    /* clean up object here */

    G_OBJECT_CLASS(gst_peconvolver_parent_class)->finalize(object);
}

static gboolean gst_peconvolver_setup(GstAudioFilter* filter,
                                      const GstAudioInfo* info) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(filter);

    GST_DEBUG_OBJECT(peconvolver, "setup");

    peconvolver->rate = info->rate;
    peconvolver->bps = GST_AUDIO_INFO_BPS(info);

    return true;
}

/* transform */
static GstFlowReturn gst_peconvolver_transform(GstBaseTransform* trans,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "transform");

    if (peconvolver->ready) {
        gst_buffer_ref(inbuf);
        gst_adapter_push(peconvolver->adapter, inbuf);

        unsigned int conv_nbytes =
            2 * peconvolver->conv_buffer_size * peconvolver->bps;

        gst_buffer_resize(outbuf, 0, 0);

        if (gst_adapter_available(peconvolver->adapter) >= conv_nbytes) {
            GstBuffer* buffer =
                gst_adapter_take_buffer(peconvolver->adapter, conv_nbytes);

            gst_buffer_append(outbuf, buffer);

            process(peconvolver, outbuf);

            // I think I should do this. But there is a crash if I do so.
            // gst_buffer_unref(buffer);
        }
    } else {
        // passthrough

        GstMapInfo map_in, map_out;

        gst_buffer_map(inbuf, &map_in, GST_MAP_READ);
        gst_buffer_map(outbuf, &map_out, GST_MAP_WRITE);

        memcpy(map_out.data, map_in.data, map_in.size);

        gst_buffer_unmap(inbuf, &map_in);
        gst_buffer_unmap(outbuf, &map_out);
    }

    return GST_FLOW_OK;
}

static GstStateChangeReturn gst_peconvolver_change_state(
    GstElement* element,
    GstStateChange transition) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(element);
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    // upwards state changes here

    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            setup_convolver(peconvolver);
            break;
        default:
            break;
    }

    ret = GST_ELEMENT_CLASS(gst_peconvolver_parent_class)
              ->change_state(element, transition);

    if (ret == GST_STATE_CHANGE_FAILURE)
        return ret;

    // downwards state changes here

    switch (transition) {
        case GST_STATE_CHANGE_READY_TO_NULL:
            finish_convolver(peconvolver);
            break;
        default:
            break;
    }

    return ret;
}

static void process(GstPeconvolver* peconvolver, GstBuffer* buffer) {
    GstMapInfo map;

    gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

    // output is always stereo. That is why we divide by 2
    guint num_samples = map.size / (2 * peconvolver->bps);

    // std::cout << "gst buffer samples: " << num_samples << std::endl;

    // deinterleave
    for (unsigned int n = 0; n < num_samples; n++) {
        peconvolver->conv->inpdata(0)[n] = ((float*)map.data)[2 * n];
        peconvolver->conv->inpdata(1)[n] = ((float*)map.data)[2 * n + 1];
    }

    int ret = peconvolver->conv->process(true);

    if (ret != 0) {
        std::cout << "IR: process failed: " << ret << std::endl;
    }

    // interleave
    for (unsigned int n = 0; n < num_samples; n++) {
        ((float*)map.data)[2 * n] = peconvolver->conv->outdata(0)[n];
        ((float*)map.data)[2 * n + 1] = peconvolver->conv->outdata(1)[n];
    }

    gst_buffer_unmap(buffer, &map);
}

static void setup_convolver(GstPeconvolver* peconvolver) {
    bool failed = false;

    rk::read_file(peconvolver);

    float density = 0.0f;
    int max_size;

    peconvolver->conv = new Convproc();

    if (peconvolver->kernel_n_frames > 0x00100000) {
        max_size = 0x00100000;
    } else {
        max_size = peconvolver->kernel_n_frames;
    }

    // std::cout << "num_samples: " << peconvolver->kernel_n_frames <<
    // std::endl;

    int ret = peconvolver->conv->configure(
        2, 2, max_size, peconvolver->conv_buffer_size,
        peconvolver->conv_buffer_size, peconvolver->conv_buffer_size, density);

    if (ret != 0) {
        failed = true;
        std::cout << "IR: can't initialise zita-convolver engine: " << ret
                  << std::endl;
    }

    ret = peconvolver->conv->impdata_create(0, 0, 1, peconvolver->kernel_L, 0,
                                            peconvolver->kernel_n_frames);

    if (ret != 0) {
        failed = true;
        std::cout << "IR: left impdata_create failed: " << ret << std::endl;
    }

    ret = peconvolver->conv->impdata_create(1, 1, 1, peconvolver->kernel_R, 0,
                                            peconvolver->kernel_n_frames);

    if (ret != 0) {
        failed = true;
        std::cout << "IR: right impdata_create failed: " << ret << std::endl;
    }

    ret = peconvolver->conv->start_process(0, 0);

    if (ret != 0) {
        failed = true;
        std::cout << "IR: start_process failed: " << ret << std::endl;
    }

    peconvolver->adapter = gst_adapter_new();

    peconvolver->ready = (failed) ? false : true;
}

static void finish_convolver(GstPeconvolver* peconvolver) {
    peconvolver->ready = false;

    if (peconvolver->conv) {
        if (peconvolver->conv->state() != Convproc::ST_STOP) {
            peconvolver->conv->stop_process();
        }

        peconvolver->conv->cleanup();

        delete peconvolver->conv;
    }

    if (peconvolver->kernel_L) {
        delete[] peconvolver->kernel_L;
    }

    if (peconvolver->kernel_R) {
        delete[] peconvolver->kernel_R;
    }

    if (peconvolver->adapter) {
        gst_adapter_clear(peconvolver->adapter);

        g_object_unref(peconvolver->adapter);
    }

    std::cout << "finishing convolver" << std::endl;
}

static gboolean plugin_init(GstPlugin* plugin) {
    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register(plugin, "peconvolver", GST_RANK_NONE,
                                GST_TYPE_PECONVOLVER);
}

#ifndef PACKAGE
#define PACKAGE "PulseEffects"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  peconvolver,
                  "PulseEffects Convolver",
                  plugin_init,
                  "0.1",
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
