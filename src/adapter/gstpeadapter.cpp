/**
 * SECTION:element-peadapter
 * @title: peadapter
 *
 * The peadapter is a simple element that drops buffers when the
 * #GstPeadapter:drop property is set to %TRUE and lets then through otherwise.
 *
 * Any downstream error received while the #GstPeadapter:drop property is %TRUE
 * is ignored. So downstream element can be set to  %GST_STATE_NULL and removed,
 * without using pad blocking.
 */

#include <string.h>
#include "config.h"
#include "gstpeadapter.hpp"

GST_DEBUG_CATEGORY_STATIC(peadapter_debug);
#define GST_CAT_DEFAULT (peadapter_debug)

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                    "channels=2,layout=interleaved"));

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                    "channels=2,layout=interleaved"));

enum { PROP_0, PROP_DROP };

#define DEFAULT_DROP FALSE

static void gst_peadapter_set_property(GObject* object,
                                       guint prop_id,
                                       const GValue* value,
                                       GParamSpec* pspec);
static void gst_peadapter_get_property(GObject* object,
                                       guint prop_id,
                                       GValue* value,
                                       GParamSpec* pspec);

static GstFlowReturn gst_peadapter_chain(GstPad* pad,
                                         GstObject* parent,
                                         GstBuffer* buffer);
static gboolean gst_peadapter_sink_event(GstPad* pad,
                                         GstObject* parent,
                                         GstEvent* event);

#define _do_init \
    GST_DEBUG_CATEGORY_INIT(peadapter_debug, "peadapter", 0, "Peadapter");
#define gst_peadapter_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstPeadapter,
                        gst_peadapter,
                        GST_TYPE_ELEMENT,
                        _do_init);

static void gst_peadapter_class_init(GstPeadapterClass* klass) {
    GObjectClass* gobject_class;
    GstElementClass* gstelement_class;

    gobject_class = (GObjectClass*)klass;
    gstelement_class = (GstElementClass*)(klass);

    gobject_class->set_property = gst_peadapter_set_property;
    gobject_class->get_property = gst_peadapter_get_property;

    g_object_class_install_property(
        gobject_class, PROP_DROP,
        g_param_spec_boolean(
            "drop", "Drop buffers and events",
            "Whether to drop buffers and events or let them through",
            DEFAULT_DROP,
            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                     GST_PARAM_MUTABLE_PLAYING |
                                     G_PARAM_STATIC_STRINGS)));

    gst_element_class_add_static_pad_template(gstelement_class, &srctemplate);
    gst_element_class_add_static_pad_template(gstelement_class, &sinktemplate);

    gst_element_class_set_static_metadata(
        gstelement_class, "Peadapter element", "Filter",
        "Gives output buffers in the desired size",
        "Wellington <wellingtonwallace@gmail.com>");
}

static void gst_peadapter_init(GstPeadapter* peadapter) {
    peadapter->drop = FALSE;
    peadapter->discont = FALSE;

    peadapter->srcpad = gst_pad_new_from_static_template(&srctemplate, "src");

    gst_element_add_pad(GST_ELEMENT(peadapter), peadapter->srcpad);

    peadapter->sinkpad =
        gst_pad_new_from_static_template(&sinktemplate, "sink");

    gst_pad_set_chain_function(peadapter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_peadapter_chain));

    gst_pad_set_event_function(peadapter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_peadapter_sink_event));

    gst_element_add_pad(GST_ELEMENT(peadapter), peadapter->sinkpad);
}

static void gst_peadapter_set_property(GObject* object,
                                       guint prop_id,
                                       const GValue* value,
                                       GParamSpec* pspec) {
    GstPeadapter* peadapter = GST_PEADAPTER(object);

    switch (prop_id) {
        case PROP_DROP:
            peadapter->drop = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gst_peadapter_get_property(GObject* object,
                                       guint prop_id,
                                       GValue* value,
                                       GParamSpec* pspec) {
    GstPeadapter* peadapter = GST_PEADAPTER(object);

    switch (prop_id) {
        case PROP_DROP:
            g_value_set_boolean(value, peadapter->drop);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static GstFlowReturn gst_peadapter_chain(GstPad* pad,
                                         GstObject* parent,
                                         GstBuffer* buffer) {
    GstPeadapter* peadapter = GST_PEADAPTER(parent);
    GstFlowReturn ret = GST_FLOW_OK;

    if (peadapter->drop) {
        gst_buffer_unref(buffer);
    } else {
        ret = gst_pad_push(peadapter->srcpad, buffer);
    }

    return ret;
}

static gboolean gst_peadapter_sink_event(GstPad* pad,
                                         GstObject* parent,
                                         GstEvent* event) {
    GstPeadapter* peadapter = GST_PEADAPTER(parent);
    gboolean ret = true;

    // gst_event_unref(event);
    ret = gst_pad_event_default(pad, parent, event);

    switch (GST_EVENT_TYPE(event)) {
        case GST_EVENT_CAPS:
            /* we should handle the format here */
            /* push the event downstream */
            ret = gst_pad_push_event(peadapter->srcpad, event);
            break;
        default:
            /* just call the default handler */
            ret = gst_pad_event_default(pad, parent, event);
            break;
    }

    return ret;
}

static gboolean plugin_init(GstPlugin* plugin) {
    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register(plugin, "peadapter", GST_RANK_NONE,
                                GST_TYPE_PEADAPTER);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  peadapter,
                  "PulseEffects Buffer Adapter",
                  plugin_init,
                  VERSION,
                  "LGPL",
                  PACKAGE,
                  "https://github.com/wwmm/pulseeffects")
