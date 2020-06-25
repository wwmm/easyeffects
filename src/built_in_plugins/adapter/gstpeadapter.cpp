#include "gstpeadapter.hpp"
#include <gst/audio/audio.h>
#include "config.h"
#include "util.hpp"

GST_DEBUG_CATEGORY_STATIC(peadapter_debug);
#define GST_CAT_DEFAULT (peadapter_debug)

static void gst_peadapter_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec);

static void gst_peadapter_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec);

static GstFlowReturn gst_peadapter_chain(GstPad* pad, GstObject* parent, GstBuffer* buffer);

static gboolean gst_peadapter_sink_event(GstPad* pad, GstObject* parent, GstEvent* event);

static GstStateChangeReturn gst_peadapter_change_state(GstElement* element, GstStateChange transition);

static gboolean gst_peadapter_src_query(GstPad* pad, GstObject* parent, GstQuery* query);

static void gst_peadapter_finalize(GObject* object);

static GstFlowReturn gst_peadapter_process(GstPeadapter* peadapter);

static GstStaticPadTemplate sinktemplate =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

static GstStaticPadTemplate srctemplate =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw,format=F32LE,rate=[1,max],"
                                            "channels=2,layout=interleaved"));

enum { PROP_BLOCKSIZE = 1 };

enum {
  BLOCKSIZE_64 = 64,
  BLOCKSIZE_128 = 128,
  BLOCKSIZE_256 = 256,
  BLOCKSIZE_512 = 512,
  BLOCKSIZE_1024 = 1024,
  BLOCKSIZE_2048 = 2048,
  BLOCKSIZE_4096 = 4096
};

#define GST_TYPE_PEADAPTER_BLOCKSIZE (gst_peadapter_blocksize_get_type())
static GType gst_peadapter_blocksize_get_type(void) {
  static GType gtype = 0;

  if (gtype == 0) {
    static const GEnumValue values[] = {
        {BLOCKSIZE_64, "Block size 64", "64"},       {BLOCKSIZE_128, "Block size 128", "128"},
        {BLOCKSIZE_256, "Block size 256", "256"},    {BLOCKSIZE_512, "Block size 512 (default)", "512"},
        {BLOCKSIZE_1024, "Block size 1024", "1024"}, {BLOCKSIZE_2048, "Block size 2048", "2048"},
        {BLOCKSIZE_4096, "Block size 4096", "4096"}, {0, NULL, NULL}};

    gtype = g_enum_register_static("GstPeadapterBlockSize", values);
  }

  return gtype;
}

#define gst_peadapter_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstPeadapter,
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

  gstelement_class->change_state = gst_peadapter_change_state;

  gobject_class->finalize = gst_peadapter_finalize;

  gst_element_class_set_static_metadata(gstelement_class, "Peadapter element", "Filter",
                                        "Gives output buffers sizes that are a power of 2",
                                        "Wellington <wellingtonwallace@gmail.com>");

  g_object_class_install_property(
      gobject_class, PROP_BLOCKSIZE,
      g_param_spec_enum("blocksize", "Block Size", "Number of Samples in the Audio Buffer",
                        GST_TYPE_PEADAPTER_BLOCKSIZE, BLOCKSIZE_512,
                        static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
}

static void gst_peadapter_init(GstPeadapter* peadapter) {
  peadapter->rate = -1;
  peadapter->bpf = -1;
  peadapter->blocksize = 512;
  peadapter->inbuf_n_samples = -1;
  peadapter->flag_discont = false;
  peadapter->adapter = gst_adapter_new();

  peadapter->srcpad = gst_pad_new_from_static_template(&srctemplate, "src");

  /* configure event function on the pad before adding the pad to the element
   */

  gst_pad_set_query_function(peadapter->srcpad, gst_peadapter_src_query);

  gst_element_add_pad(GST_ELEMENT(peadapter), peadapter->srcpad);

  peadapter->sinkpad = gst_pad_new_from_static_template(&sinktemplate, "sink");

  gst_pad_set_chain_function(peadapter->sinkpad, GST_DEBUG_FUNCPTR(gst_peadapter_chain));

  gst_pad_set_event_function(peadapter->sinkpad, GST_DEBUG_FUNCPTR(gst_peadapter_sink_event));

  gst_element_add_pad(GST_ELEMENT(peadapter), peadapter->sinkpad);
}

static void gst_peadapter_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  GstPeadapter* peadapter = GST_PEADAPTER(object);

  switch (prop_id) {
    case PROP_BLOCKSIZE:
      peadapter->blocksize = g_value_get_enum(value);

      gst_element_post_message(GST_ELEMENT_CAST(peadapter), gst_message_new_latency(GST_OBJECT_CAST(peadapter)));

      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void gst_peadapter_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  GstPeadapter* peadapter = GST_PEADAPTER(object);

  switch (prop_id) {
    case PROP_BLOCKSIZE:
      g_value_set_enum(value, peadapter->blocksize);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static GstFlowReturn gst_peadapter_chain(GstPad* pad, GstObject* parent, GstBuffer* buffer) {
  GstPeadapter* peadapter = GST_PEADAPTER(parent);
  GstFlowReturn ret = GST_FLOW_OK;

  if (GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_DISCONT)) {
    gst_adapter_clear(peadapter->adapter);

    peadapter->inbuf_n_samples = -1;

    peadapter->flag_discont = true;
  }

  if (peadapter->inbuf_n_samples == -1) {
    GstMapInfo map;

    gst_buffer_map(buffer, &map, GST_MAP_READ);

    peadapter->inbuf_n_samples = map.size / peadapter->bpf;

    util::debug("peadapter: pulseaudio block size " + std::to_string(peadapter->inbuf_n_samples) + " frames");

    util::debug("peadapter: we will read in chunks of " + std::to_string(peadapter->blocksize) + " frames");

    gst_buffer_unmap(buffer, &map);
  }

  gst_adapter_push(peadapter->adapter, buffer);

  ret = gst_peadapter_process(peadapter);

  return ret;
}

static GstFlowReturn gst_peadapter_process(GstPeadapter* peadapter) {
  GstFlowReturn ret = GST_FLOW_OK;

  gsize nbytes = peadapter->blocksize * peadapter->bpf;
  auto duration = GST_FRAMES_TO_CLOCK_TIME(peadapter->blocksize, peadapter->rate);

  while (gst_adapter_available(peadapter->adapter) > nbytes && (ret == GST_FLOW_OK)) {
    GstBuffer* b = gst_adapter_take_buffer(peadapter->adapter, nbytes);

    if (b != nullptr) {
      b = gst_buffer_make_writable(b);

      GST_BUFFER_OFFSET(b) = gst_adapter_prev_offset(peadapter->adapter, nullptr);
      GST_BUFFER_PTS(b) = gst_adapter_prev_pts(peadapter->adapter, nullptr);
      GST_BUFFER_DURATION(b) = duration;

      if (peadapter->flag_discont) {
        gst_buffer_set_flags(b, GST_BUFFER_FLAG_DISCONT);
        gst_buffer_set_flags(b, GST_BUFFER_FLAG_RESYNC);

        peadapter->flag_discont = false;
      } else {
        gst_buffer_unset_flags(b, GST_BUFFER_FLAG_DISCONT);
      }

      gst_buffer_set_flags(b, GST_BUFFER_FLAG_NON_DROPPABLE);
      gst_buffer_set_flags(b, GST_BUFFER_FLAG_LIVE);

      ret = gst_pad_push(peadapter->srcpad, b);
    }
  }

  return ret;
}

static gboolean gst_peadapter_sink_event(GstPad* pad, GstObject* parent, GstEvent* event) {
  GstPeadapter* peadapter = GST_PEADAPTER(parent);
  gboolean ret = true;

  switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS:
      GstCaps* caps;
      GstAudioInfo info;

      gst_event_parse_caps(event, &caps);

      gst_audio_info_from_caps(&info, caps);

      peadapter->rate = GST_AUDIO_INFO_RATE(&info);
      peadapter->bpf = GST_AUDIO_INFO_BPF(&info);

      /* push the event downstream */

      ret = gst_pad_push_event(peadapter->srcpad, event);

      break;
    case GST_EVENT_FLUSH_START:
      gst_peadapter_process(peadapter);
      gst_adapter_clear(peadapter->adapter);

      peadapter->inbuf_n_samples = -1;

      ret = gst_pad_push_event(peadapter->srcpad, event);

      break;
    case GST_EVENT_EOS:
      gst_peadapter_process(peadapter);
      gst_adapter_clear(peadapter->adapter);

      peadapter->inbuf_n_samples = -1;

      ret = gst_pad_push_event(peadapter->srcpad, event);

      break;
    default:
      ret = gst_pad_push_event(peadapter->srcpad, event);
      break;
  }

  return ret;
}

static GstStateChangeReturn gst_peadapter_change_state(GstElement* element, GstStateChange transition) {
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstPeadapter* peadapter = GST_PEADAPTER(element);

  /*up changes*/

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      break;
    default:
      break;
  }

  /*down changes*/

  ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_adapter_clear(peadapter->adapter);

      peadapter->inbuf_n_samples = -1;

      break;
    default:
      break;
  }

  return ret;
}

static gboolean gst_peadapter_src_query(GstPad* pad, GstObject* parent, GstQuery* query) {
  GstPeadapter* peadapter = GST_PEADAPTER(parent);
  bool ret = true;

  switch (GST_QUERY_TYPE(query)) {
    case GST_QUERY_LATENCY:
      if (peadapter->rate > 0) {
        ret = gst_pad_peer_query(peadapter->sinkpad, query);

        if (ret && peadapter->inbuf_n_samples != -1 && peadapter->inbuf_n_samples < peadapter->blocksize) {
          GstClockTime min, max;
          gboolean live;
          guint64 latency;

          gst_query_parse_latency(query, &live, &min, &max);

          /* add our own latency */

          latency = gst_util_uint64_scale_round(peadapter->blocksize - peadapter->inbuf_n_samples, GST_SECOND,
                                                peadapter->rate);

          // std::cout << "latency: " << latency << std::endl;
          // std::cout << "n: " << peadapter->inbuf_n_samples
          //           << std::endl;

          min += latency;

          if (max != GST_CLOCK_TIME_NONE) {
            max += latency;
          }

          // std::cout << min << "\t" << max << "\t" << live
          //           << std::endl;

          gst_query_set_latency(query, live, min, max);
        }

      } else {
        ret = false;
      }

      break;
    default:
      /* just call the default handler */
      ret = gst_pad_query_default(pad, parent, query);
      break;
  }

  return ret;
}

void gst_peadapter_finalize(GObject* object) {
  GstPeadapter* peadapter = GST_PEADAPTER(object);

  GST_DEBUG_OBJECT(peadapter, "finalize");

  gst_adapter_clear(peadapter->adapter);
  g_object_unref(peadapter->adapter);

  /* clean up object here */

  G_OBJECT_CLASS(gst_peadapter_parent_class)->finalize(object);
}

static gboolean plugin_init(GstPlugin* plugin) {
  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register(plugin, "peadapter", GST_RANK_NONE, GST_TYPE_PEADAPTER);
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
