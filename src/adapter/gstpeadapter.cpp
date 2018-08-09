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

enum { PROP_0, PROP_BLOCKSIZE };

#define gst_peadapter_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(
    GstPeadapter,
    gst_peadapter,
    GST_TYPE_ELEMENT,
    GST_DEBUG_CATEGORY_INIT(peadapter_debug, "peadapter", 0, "Peadapter"));

static void gst_peadapter_class_init(GstPeadapterClass* klass) {
    GObjectClass* gobject_class;
    GstElementClass* gstelement_class;

    gobject_class = (GObjectClass*)klass;
    gstelement_class = (GstElementClass*)(klass);

    gobject_class->set_property = gst_peadapter_set_property;
    gobject_class->get_property = gst_peadapter_get_property;

    gst_element_class_add_static_pad_template(gstelement_class, &srctemplate);
    gst_element_class_add_static_pad_template(gstelement_class, &sinktemplate);

    gst_element_class_set_static_metadata(
        gstelement_class, "Peadapter element", "Filter",
        "Gives output buffers in the desired size",
        "Wellington <wellingtonwallace@gmail.com>");

    g_object_class_install_property(
        gobject_class, PROP_BLOCKSIZE,
        g_param_spec_int("blocksize", "Block Size",
                         "Number of Samples in the buffer", 0, 2048, 256,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));
}

static void gst_peadapter_init(GstPeadapter* peadapter) {
    peadapter->blocksize = 256;
    peadapter->adapter = gst_adapter_new();

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
        case PROP_BLOCKSIZE:
            peadapter->blocksize = g_value_get_int(value);
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
        case PROP_BLOCKSIZE:
            g_value_set_int(value, peadapter->blocksize);
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

    gst_adapter_push(peadapter->adapter, buffer);

    gsize nbytes = 2 * peadapter->blocksize * sizeof(float);

    if (gst_adapter_available(peadapter->adapter) >= nbytes) {
        GstBuffer* b = gst_adapter_take_buffer(peadapter->adapter, nbytes);

        return gst_pad_push(peadapter->srcpad, b);
    } else {
        return GST_FLOW_OK;
    }
}

static gboolean gst_peadapter_sink_event(GstPad* pad,
                                         GstObject* parent,
                                         GstEvent* event) {
    GstPeadapter* peadapter = GST_PEADAPTER(parent);
    gboolean ret = true;

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
