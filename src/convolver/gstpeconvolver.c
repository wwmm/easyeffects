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

#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include "gstpeconvolver.h"

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

static gboolean gst_peconvolver_setup(GstAudioFilter* filter,
                                      const GstAudioInfo* info);
static GstFlowReturn gst_peconvolver_transform(GstBaseTransform* trans,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf);
static GstFlowReturn gst_peconvolver_transform_ip(GstBaseTransform* trans,
                                                  GstBuffer* buf);

enum { PROP_0 };

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

    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */
    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peconvolver_src_template);
    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass),
                                              &gst_peconvolver_sink_template);

    gst_element_class_set_static_metadata(
        GST_ELEMENT_CLASS(klass), "PulseEffects Convolver", "Generic",
        "PulseEffects Convolver", "Wellington <wellingtonwallace@gmail.com>");

    gobject_class->set_property = gst_peconvolver_set_property;
    gobject_class->get_property = gst_peconvolver_get_property;
    gobject_class->dispose = gst_peconvolver_dispose;
    gobject_class->finalize = gst_peconvolver_finalize;
    audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_peconvolver_setup);
    base_transform_class->transform =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform);
    base_transform_class->transform_ip =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform_ip);
}

static void gst_peconvolver_init(GstPeconvolver* peconvolver) {}

void gst_peconvolver_set_property(GObject* object,
                                  guint property_id,
                                  const GValue* value,
                                  GParamSpec* pspec) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "set_property");

    switch (property_id) {
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

    G_OBJECT_CLASS(gst_peconvolver_parent_class)->finalize(object);
}

static gboolean gst_peconvolver_setup(GstAudioFilter* filter,
                                      const GstAudioInfo* info) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(filter);

    GST_DEBUG_OBJECT(peconvolver, "setup");

    return TRUE;
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
