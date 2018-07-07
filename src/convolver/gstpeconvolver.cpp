/**
 * SECTION:element-gstpeconvolver
 *
 * The peconvolver element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! peconvolver ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/base/gstbasetransform.h>
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

static void gst_peconvolver_dispose(GObject* object);

static void gst_peconvolver_finalize(GObject* object);

static GstFlowReturn gst_peconvolver_chain(GstPad* pad,
                                           GstObject* parent,
                                           GstBuffer* buf);

enum { PROP_0, PROP_KERNEL_PATH };

/* pad templates */

static GstStaticPadTemplate gst_peconvolver_src_template =
    GST_STATIC_PAD_TEMPLATE(
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                        "channels=2,layout=interleaved"));

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
    GST_TYPE_BASE_TRANSFORM,
    GST_DEBUG_CATEGORY_INIT(gst_peconvolver_debug_category,
                            "peconvolver",
                            0,
                            "debug category for peconvolver element"));

static void gst_peconvolver_class_init(GstPeconvolverClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    // GstBaseTransformClass* base_transform_class =
    //     GST_BASE_TRANSFORM_CLASS(klass);

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

    gobject_class->dispose = gst_peconvolver_dispose;
    gobject_class->finalize = gst_peconvolver_finalize;

    /* define properties */

    g_object_class_install_property(
        gobject_class, PROP_KERNEL_PATH,
        g_param_spec_string("kernelpath", "Kernel Path",
                            "Full path to kernel file", nullptr,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS)));
}

static void gst_peconvolver_init(GstPeconvolver* peconvolver) {
    // add sinkpad

    peconvolver->sinkpad = gst_pad_new_from_static_template(
        &gst_peconvolver_sink_template, nullptr);

    /* configure chain function on the pad before adding
     * the pad to the element */
    gst_pad_set_chain_function(peconvolver->sinkpad, gst_peconvolver_chain);

    gst_element_add_pad(GST_ELEMENT(peconvolver), peconvolver->sinkpad);

    // add source pad

    peconvolver->srcpad = gst_pad_new_from_static_template(
        &gst_peconvolver_src_template, nullptr);

    gst_element_add_pad(GST_ELEMENT(peconvolver), peconvolver->srcpad);

    peconvolver->conv_buffer_size = 1024;

    // peconvolver->adapter = gst_adapter_new();
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

void gst_peconvolver_dispose(GObject* object) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "dispose");

    /* clean up as possible.  may be called multiple times */

    G_OBJECT_CLASS(gst_peconvolver_parent_class)->dispose(object);
}

void gst_peconvolver_finalize(GObject* object) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "finalize");

    /* clean up object here */

    // if (peconvolver->conv->state() != Convproc::ST_STOP) {
    //     peconvolver->conv->stop_process();
    // }
    //
    // peconvolver->conv->cleanup();
    //
    // g_object_unref(peconvolver->adapter);
    //
    // delete peconvolver->conv;
    // delete[] peconvolver->kernel_L;
    // delete[] peconvolver->kernel_R;

    G_OBJECT_CLASS(gst_peconvolver_parent_class)->finalize(object);
}

static GstFlowReturn gst_peconvolver_chain(GstPad* pad,
                                           GstObject* parent,
                                           GstBuffer* buf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(parent);

    return gst_pad_push(peconvolver->srcpad, buf);
}

static gboolean plugin_init(GstPlugin* plugin) {
    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register(plugin, "peconvolver", GST_RANK_NONE,
                                GST_TYPE_PECONVOLVER);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */

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
                  "PulseEffects",
                  "https://github.com/wwmm/pulseeffects")
