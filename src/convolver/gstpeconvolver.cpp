/* GStreamer
 * Copyright (C) 2018 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
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

static GstCaps* gst_peconvolver_transform_caps(GstBaseTransform* trans,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* filter);

static GstCaps* gst_peconvolver_fixate_caps(GstBaseTransform* trans,
                                            GstPadDirection direction,
                                            GstCaps* caps,
                                            GstCaps* othercaps);

static gboolean gst_peconvolver_accept_caps(GstBaseTransform* trans,
                                            GstPadDirection direction,
                                            GstCaps* caps);

static gboolean gst_peconvolver_set_caps(GstBaseTransform* trans,
                                         GstCaps* incaps,
                                         GstCaps* outcaps);

static gboolean gst_peconvolver_query(GstBaseTransform* trans,
                                      GstPadDirection direction,
                                      GstQuery* query);

static gboolean gst_peconvolver_decide_allocation(GstBaseTransform* trans,
                                                  GstQuery* query);

static gboolean gst_peconvolver_filter_meta(GstBaseTransform* trans,
                                            GstQuery* query,
                                            GType api,
                                            const GstStructure* params);

static gboolean gst_peconvolver_propose_allocation(GstBaseTransform* trans,
                                                   GstQuery* decide_query,
                                                   GstQuery* query);

static gboolean gst_peconvolver_transform_size(GstBaseTransform* trans,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               gsize size,
                                               GstCaps* othercaps,
                                               gsize* othersize);

static gboolean gst_peconvolver_get_unit_size(GstBaseTransform* trans,
                                              GstCaps* caps,
                                              gsize* size);

static gboolean gst_peconvolver_start(GstBaseTransform* trans);

static gboolean gst_peconvolver_stop(GstBaseTransform* trans);

static gboolean gst_peconvolver_sink_event(GstBaseTransform* trans,
                                           GstEvent* event);

static gboolean gst_peconvolver_src_event(GstBaseTransform* trans,
                                          GstEvent* event);

static GstFlowReturn gst_peconvolver_prepare_output_buffer(
    GstBaseTransform* trans,
    GstBuffer* input,
    GstBuffer** outbuf);

static gboolean gst_peconvolver_copy_metadata(GstBaseTransform* trans,
                                              GstBuffer* input,
                                              GstBuffer* outbuf);

static gboolean gst_peconvolver_transform_meta(GstBaseTransform* trans,
                                               GstBuffer* outbuf,
                                               GstMeta* meta,
                                               GstBuffer* inbuf);

static void gst_peconvolver_before_transform(GstBaseTransform* trans,
                                             GstBuffer* buffer);

static GstFlowReturn gst_peconvolver_transform(GstBaseTransform* trans,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf);

static GstFlowReturn gst_peconvolver_transform_ip(GstBaseTransform* trans,
                                                  GstBuffer* buf);

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

    GstBaseTransformClass* base_transform_class =
        GST_BASE_TRANSFORM_CLASS(klass);

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

    base_transform_class->transform_caps =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform_caps);

    base_transform_class->fixate_caps =
        GST_DEBUG_FUNCPTR(gst_peconvolver_fixate_caps);

    base_transform_class->accept_caps =
        GST_DEBUG_FUNCPTR(gst_peconvolver_accept_caps);

    base_transform_class->set_caps =
        GST_DEBUG_FUNCPTR(gst_peconvolver_set_caps);

    base_transform_class->query = GST_DEBUG_FUNCPTR(gst_peconvolver_query);

    base_transform_class->decide_allocation =
        GST_DEBUG_FUNCPTR(gst_peconvolver_decide_allocation);

    base_transform_class->filter_meta =
        GST_DEBUG_FUNCPTR(gst_peconvolver_filter_meta);

    base_transform_class->propose_allocation =
        GST_DEBUG_FUNCPTR(gst_peconvolver_propose_allocation);

    base_transform_class->transform_size =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform_size);

    base_transform_class->get_unit_size =
        GST_DEBUG_FUNCPTR(gst_peconvolver_get_unit_size);

    base_transform_class->start = GST_DEBUG_FUNCPTR(gst_peconvolver_start);

    base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_peconvolver_stop);

    base_transform_class->sink_event =
        GST_DEBUG_FUNCPTR(gst_peconvolver_sink_event);

    base_transform_class->src_event =
        GST_DEBUG_FUNCPTR(gst_peconvolver_src_event);

    base_transform_class->prepare_output_buffer =
        GST_DEBUG_FUNCPTR(gst_peconvolver_prepare_output_buffer);

    base_transform_class->copy_metadata =
        GST_DEBUG_FUNCPTR(gst_peconvolver_copy_metadata);

    base_transform_class->transform_meta =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform_meta);

    base_transform_class->before_transform =
        GST_DEBUG_FUNCPTR(gst_peconvolver_before_transform);

    base_transform_class->transform =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform);

    base_transform_class->transform_ip =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform_ip);

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
        &gst_peconvolver_sink_template, "sink");

    /* configure chain function on the pad before adding
     * the pad to the element */
    gst_pad_set_chain_function(peconvolver->sinkpad, gst_peconvolver_chain);

    gst_element_add_pad(GST_ELEMENT(peconvolver), peconvolver->sinkpad);

    // add source pad

    peconvolver->srcpad =
        gst_pad_new_from_static_template(&gst_peconvolver_src_template, "src");

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

    if (peconvolver->conv->state() != Convproc::ST_STOP) {
        peconvolver->conv->stop_process();
    }

    peconvolver->conv->cleanup();

    g_object_unref(peconvolver->adapter);

    delete peconvolver->conv;
    delete[] peconvolver->kernel_L;
    delete[] peconvolver->kernel_R;

    G_OBJECT_CLASS(gst_peconvolver_parent_class)->finalize(object);
}

static GstCaps* gst_peconvolver_transform_caps(GstBaseTransform* trans,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               GstCaps* filter) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);
    GstCaps* othercaps;

    GST_DEBUG_OBJECT(peconvolver, "transform_caps");

    othercaps = gst_caps_copy(caps);

    /* Copy other caps and modify as appropriate */
    /* This works for the simplest cases, where the transform modifies one
     * or more fields in the caps structure.  It does not work correctly
     * if passthrough caps are preferred. */
    if (direction == GST_PAD_SRC) {
        /* transform caps going upstream */
    } else {
        /* transform caps going downstream */
    }

    if (filter) {
        GstCaps* intersect;

        intersect = gst_caps_intersect(othercaps, filter);
        gst_caps_unref(othercaps);

        return intersect;
    } else {
        return othercaps;
    }
}

static GstCaps* gst_peconvolver_fixate_caps(GstBaseTransform* trans,
                                            GstPadDirection direction,
                                            GstCaps* caps,
                                            GstCaps* othercaps) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "fixate_caps");

    return NULL;
}

static gboolean gst_peconvolver_accept_caps(GstBaseTransform* trans,
                                            GstPadDirection direction,
                                            GstCaps* caps) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "accept_caps");

    return TRUE;
}

static gboolean gst_peconvolver_set_caps(GstBaseTransform* trans,
                                         GstCaps* incaps,
                                         GstCaps* outcaps) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "set_caps");

    return TRUE;
}

static gboolean gst_peconvolver_query(GstBaseTransform* trans,
                                      GstPadDirection direction,
                                      GstQuery* query) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "query");

    return TRUE;
}

/* decide allocation query for output buffers */
static gboolean gst_peconvolver_decide_allocation(GstBaseTransform* trans,
                                                  GstQuery* query) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "decide_allocation");

    return TRUE;
}

static gboolean gst_peconvolver_filter_meta(GstBaseTransform* trans,
                                            GstQuery* query,
                                            GType api,
                                            const GstStructure* params) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "filter_meta");

    return TRUE;
}

/* propose allocation query parameters for input buffers */
static gboolean gst_peconvolver_propose_allocation(GstBaseTransform* trans,
                                                   GstQuery* decide_query,
                                                   GstQuery* query) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "propose_allocation");

    return TRUE;
}

/* transform size */
static gboolean gst_peconvolver_transform_size(GstBaseTransform* trans,
                                               GstPadDirection direction,
                                               GstCaps* caps,
                                               gsize size,
                                               GstCaps* othercaps,
                                               gsize* othersize) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "transform_size");

    return TRUE;
}

static gboolean gst_peconvolver_get_unit_size(GstBaseTransform* trans,
                                              GstCaps* caps,
                                              gsize* size) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "get_unit_size");

    return TRUE;
}

/* states */
static gboolean gst_peconvolver_start(GstBaseTransform* trans) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "start");

    return TRUE;
}

static gboolean gst_peconvolver_stop(GstBaseTransform* trans) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "stop");

    return TRUE;
}

/* sink and src pad event handlers */
static gboolean gst_peconvolver_sink_event(GstBaseTransform* trans,
                                           GstEvent* event) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "sink_event");

    return GST_BASE_TRANSFORM_CLASS(gst_peconvolver_parent_class)
        ->sink_event(trans, event);
}

static gboolean gst_peconvolver_src_event(GstBaseTransform* trans,
                                          GstEvent* event) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "src_event");

    return GST_BASE_TRANSFORM_CLASS(gst_peconvolver_parent_class)
        ->src_event(trans, event);
}

static GstFlowReturn gst_peconvolver_prepare_output_buffer(
    GstBaseTransform* trans,
    GstBuffer* input,
    GstBuffer** outbuf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "prepare_output_buffer");

    return GST_FLOW_OK;
}

/* metadata */
static gboolean gst_peconvolver_copy_metadata(GstBaseTransform* trans,
                                              GstBuffer* input,
                                              GstBuffer* outbuf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "copy_metadata");

    return TRUE;
}

static gboolean gst_peconvolver_transform_meta(GstBaseTransform* trans,
                                               GstBuffer* outbuf,
                                               GstMeta* meta,
                                               GstBuffer* inbuf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "transform_meta");

    return TRUE;
}

static void gst_peconvolver_before_transform(GstBaseTransform* trans,
                                             GstBuffer* buffer) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "before_transform");
}

/* transform */
static GstFlowReturn gst_peconvolver_transform(GstBaseTransform* trans,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "transform");

    return GST_FLOW_OK;
}

static GstFlowReturn gst_peconvolver_transform_ip(GstBaseTransform* trans,
                                                  GstBuffer* buf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "transform_ip");

    return GST_FLOW_OK;
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
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  peconvolver,
                  "FIXME plugin description",
                  plugin_init,
                  VERSION,
                  "LGPL",
                  PACKAGE_NAME,
                  GST_PACKAGE_ORIGIN)
