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

static gboolean gst_peconvolver_stop(GstBaseTransform* base);

static gboolean gst_peconvolver_query(GstBaseTransform* trans,
                                      GstPadDirection direction,
                                      GstQuery* query);

static void gst_peconvolver_set_kernel_path(GstPeconvolver* peconvolver,
                                            gchar* value);

static void gst_peconvolver_set_buffersize(GstPeconvolver* peconvolver,
                                           const uint& value);

static void gst_peconvolver_set_ir_width(GstPeconvolver* peconvolver,
                                         const uint& value);

static void gst_peconvolver_process(GstPeconvolver* peconvolver,
                                    GstBuffer* buffer);

static void gst_peconvolver_setup_convolver(GstPeconvolver* peconvolver);

static void gst_peconvolver_finish_convolver(GstPeconvolver* peconvolver);

/*global variables and my defines*/

#define MAX_IRS_FRAMES 0x00100000

// taken from https://github.com/x42/convoLV2/blob/master/convolution.cc
/*
 * Priority should match -P parameter passed to jackd.
 * Sched.class: either SCHED_FIFO or SCHED_RR (I think Jack uses SCHED_FIFO).
 *
 * THREAD_SYNC_MODE must be true if you want to use the plugin in Jack
 * freewheeling mode (eg. while exporting in Ardour). You may only use
 * false if you *only* run the plugin realtime.
 */
#define CONVPROC_SCHEDULER_PRIORITY 0
#define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
#define THREAD_SYNC_MODE true

#define GST_TYPE_PECONVOLVER_BUFFER_SIZE \
    (gst_peconvolver_buffer_size_get_type())
static GType gst_peconvolver_buffer_size_get_type(void) {
    static GType buffer_size_type = 0;
    static const GEnumValue buffer_size[] = {
        {GST_PECONVOLVER_BUFFER_SIZE_DEFAULT, "Default size", "256"},
        {GST_PECONVOLVER_BUFFER_SIZE_64, "64", "64"},
        {GST_PECONVOLVER_BUFFER_SIZE_128, "128", "128"},
        {GST_PECONVOLVER_BUFFER_SIZE_256, "256", "256"},
        {GST_PECONVOLVER_BUFFER_SIZE_512, "512", "512"},
        {GST_PECONVOLVER_BUFFER_SIZE_1024, "1024", "1024"},
        {GST_PECONVOLVER_BUFFER_SIZE_2048, "2048", "2048"},
        {GST_PECONVOLVER_BUFFER_SIZE_4096, "4096", "4096"},
        {GST_PECONVOLVER_BUFFER_SIZE_8192, "8192", "8192"},
        {0, NULL, NULL},
    };

    if (!buffer_size_type) {
        buffer_size_type =
            g_enum_register_static("GstPeconvolverBufferSize", buffer_size);
    }
    return buffer_size_type;
}

enum { PROP_0, PROP_KERNEL_PATH, PROP_BUFFER_SIZE, PROP_IR_WIDTH };

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

    /* define virtual function pointers */

    gobject_class->set_property = gst_peconvolver_set_property;
    gobject_class->get_property = gst_peconvolver_get_property;

    gobject_class->finalize = gst_peconvolver_finalize;

    audio_filter_class->setup = GST_DEBUG_FUNCPTR(gst_peconvolver_setup);

    base_transform_class->transform =
        GST_DEBUG_FUNCPTR(gst_peconvolver_transform);

    base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_peconvolver_stop);

    base_transform_class->query = GST_DEBUG_FUNCPTR(gst_peconvolver_query);

    /* define properties */

    g_object_class_install_property(
        gobject_class, PROP_KERNEL_PATH,
        g_param_spec_string("kernel-path", "Kernel Path",
                            "Full path to kernel file", nullptr,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_BUFFER_SIZE,
        g_param_spec_enum("buffersize", "Buffer Size",
                          "Zita Convolver Partition Size",
                          GST_TYPE_PECONVOLVER_BUFFER_SIZE,
                          GST_PECONVOLVER_BUFFER_SIZE_DEFAULT,
                          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                   G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_IR_WIDTH,
        g_param_spec_int("ir-width", "IR Width", "ImpulseResponse Stereo Width",
                         0, 200, 100,
                         static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                  G_PARAM_STATIC_STRINGS)));
}

static void gst_peconvolver_init(GstPeconvolver* peconvolver) {
    peconvolver->log_tag = "convolver: ";
    peconvolver->ready = false;
    peconvolver->rate = 0;
    peconvolver->bpf = 0;
    peconvolver->buffer_size = GST_PECONVOLVER_BUFFER_SIZE_DEFAULT;
    peconvolver->kernel_path = nullptr;
    peconvolver->ir_width = 100;
}

void gst_peconvolver_set_property(GObject* object,
                                  guint property_id,
                                  const GValue* value,
                                  GParamSpec* pspec) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "set_property");

    switch (property_id) {
        case PROP_KERNEL_PATH:
            gst_peconvolver_set_kernel_path(peconvolver,
                                            g_value_dup_string(value));

            break;
        case PROP_BUFFER_SIZE:
            gst_peconvolver_set_buffersize(peconvolver,
                                           g_value_get_enum(value));
            break;
        case PROP_IR_WIDTH:
            gst_peconvolver_set_ir_width(peconvolver, g_value_get_int(value));
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
        case PROP_BUFFER_SIZE:
            g_value_set_enum(value, peconvolver->buffer_size);
            break;
        case PROP_IR_WIDTH:
            g_value_set_int(value, peconvolver->ir_width);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

void gst_peconvolver_finalize(GObject* object) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(object);

    GST_DEBUG_OBJECT(peconvolver, "finalize");

    std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

    gst_peconvolver_finish_convolver(peconvolver);

    /* clean up object here */

    G_OBJECT_CLASS(gst_peconvolver_parent_class)->finalize(object);
}

static gboolean gst_peconvolver_setup(GstAudioFilter* filter,
                                      const GstAudioInfo* info) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(filter);

    GST_DEBUG_OBJECT(peconvolver, "setup");

    peconvolver->rate = info->rate;
    peconvolver->bpf = GST_AUDIO_INFO_BPF(info);

    /*
    this function is called whenever there is a format change. So we reset the
    zita convolver. Setup will reinitialize zita when necessary.
    */

    std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

    gst_peconvolver_finish_convolver(peconvolver);

    return true;
}

/* transform */
static GstFlowReturn gst_peconvolver_transform(GstBaseTransform* trans,
                                               GstBuffer* inbuf,
                                               GstBuffer* outbuf) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);

    GST_DEBUG_OBJECT(peconvolver, "transform");

    if (G_UNLIKELY(GST_BUFFER_FLAG_IS_SET(inbuf, GST_BUFFER_FLAG_GAP))) {
        return GST_FLOW_OK;
    }

    std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

    gst_peconvolver_setup_convolver(peconvolver);

    if (peconvolver->ready) {
        gst_buffer_ref(inbuf);
        gst_adapter_push(peconvolver->adapter, inbuf);

        uint conv_nbytes = peconvolver->buffer_size * peconvolver->bpf;

        gst_buffer_resize(outbuf, 0, 0);
        gst_buffer_remove_all_memory(outbuf);

        while (gst_adapter_available(peconvolver->adapter) >= conv_nbytes) {
            GstBuffer* buffer =
                gst_adapter_take_buffer(peconvolver->adapter, conv_nbytes);

            gst_peconvolver_process(peconvolver, buffer);

            outbuf = gst_buffer_append(outbuf, buffer);
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

static gboolean gst_peconvolver_stop(GstBaseTransform* base) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(base);

    std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

    gst_peconvolver_finish_convolver(peconvolver);

    return true;
}

static gboolean gst_peconvolver_query(GstBaseTransform* trans,
                                      GstPadDirection direction,
                                      GstQuery* query) {
    GstPeconvolver* peconvolver = GST_PECONVOLVER(trans);
    gboolean res = true;

    /*
    most of this code was taken from
    https://github.com/Kurento/gst-plugins-good/blob/master/gst/audiofx/audiofxbasefirfilter.c
    */

    switch (GST_QUERY_TYPE(query)) {
        case GST_QUERY_LATENCY: {
            GstClockTime min, max;
            gboolean live;
            guint64 latency = peconvolver->buffer_size;
            gint rate = GST_AUDIO_FILTER_RATE(peconvolver);

            if (rate == 0) {
                res = false;
            } else if ((res = gst_pad_peer_query(
                            GST_BASE_TRANSFORM(peconvolver)->sinkpad, query))) {
                gst_query_parse_latency(query, &live, &min, &max);

                GST_DEBUG_OBJECT(peconvolver,
                                 "Peer latency: min %" GST_TIME_FORMAT
                                 " max %" GST_TIME_FORMAT,
                                 GST_TIME_ARGS(min), GST_TIME_ARGS(max));

                /* add our own latency */
                latency =
                    gst_util_uint64_scale_round(latency, GST_SECOND, rate);

                GST_DEBUG_OBJECT(peconvolver, "Our latency: %" GST_TIME_FORMAT,
                                 GST_TIME_ARGS(latency));

                min += latency;
                if (max != GST_CLOCK_TIME_NONE)
                    max += latency;

                GST_DEBUG_OBJECT(
                    peconvolver,
                    "Calculated total latency : min %" GST_TIME_FORMAT
                    " max %" GST_TIME_FORMAT,
                    GST_TIME_ARGS(min), GST_TIME_ARGS(max));

                gst_query_set_latency(query, live, min, max);
            }
            break;
        }
        default:
            res = GST_BASE_TRANSFORM_CLASS(gst_peconvolver_parent_class)
                      ->query(trans, direction, query);
            break;
    }

    return res;
}

static void gst_peconvolver_set_kernel_path(GstPeconvolver* peconvolver,
                                            gchar* value) {
    if (value != nullptr) {
        if (peconvolver->kernel_path != nullptr) {
            std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

            std::string old_path = peconvolver->kernel_path;

            g_free(peconvolver->kernel_path);

            peconvolver->kernel_path = value;

            if (peconvolver->kernel_path != nullptr) {
                if (old_path != peconvolver->kernel_path) {
                    // resetting zita
                    gst_peconvolver_finish_convolver(peconvolver);
                }
            }
        } else {
            // plugin is being initialized

            peconvolver->kernel_path = value;
        }
    }
}

static void gst_peconvolver_set_buffersize(GstPeconvolver* peconvolver,
                                           const uint& value) {
    if (value != peconvolver->buffer_size) {
        std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

        peconvolver->buffer_size = value;

        if (peconvolver->ready) {
            // resetting zita
            gst_peconvolver_finish_convolver(peconvolver);
        }
    }
}

static void gst_peconvolver_set_ir_width(GstPeconvolver* peconvolver,
                                         const uint& value) {
    if (value != peconvolver->ir_width) {
        std::lock_guard<std::mutex> lock(peconvolver->lock_guard_zita);

        peconvolver->ir_width = value;

        if (peconvolver->ready) {
            // resetting zita
            gst_peconvolver_finish_convolver(peconvolver);
        }
    }
}

static void gst_peconvolver_setup_convolver(GstPeconvolver* peconvolver) {
    if (!peconvolver->ready && peconvolver->rate != 0 &&
        peconvolver->bpf != 0) {
        util::debug(peconvolver->log_tag + "maximum irs frames supported: " +
                    std::to_string(MAX_IRS_FRAMES));

        bool irs_ok = rk::read_file(peconvolver);

        if (irs_ok) {
            bool failed = false;
            float density = 0.0f;
            int max_size;

            peconvolver->conv = new Convproc();

            if (peconvolver->kernel_n_frames > MAX_IRS_FRAMES) {
                max_size = MAX_IRS_FRAMES;
            } else {
                max_size = peconvolver->kernel_n_frames;
            }

            unsigned int options = 0;

            options |= Convproc::OPT_FFTW_MEASURE;
            options |= Convproc::OPT_VECTOR_MODE;

            peconvolver->conv->set_options(options);

            int ret = peconvolver->conv->configure(
                2, 2, max_size, peconvolver->buffer_size,
                peconvolver->buffer_size, Convproc::MAXPART, density);

            if (ret != 0) {
                failed = true;
                util::warning(peconvolver->log_tag +
                              "can't initialise zita-convolver engine: " +
                              std::to_string(ret));
            }

            ret = peconvolver->conv->impdata_create(
                0, 0, 1, peconvolver->kernel_L, 0,
                peconvolver->kernel_n_frames);

            if (ret != 0) {
                failed = true;
                util::warning(
                    peconvolver->log_tag +
                    "left impdata_create failed: " + std::to_string(ret));
            }

            ret = peconvolver->conv->impdata_create(
                1, 1, 1, peconvolver->kernel_R, 0,
                peconvolver->kernel_n_frames);

            if (ret != 0) {
                failed = true;
                util::warning(
                    peconvolver->log_tag +
                    "right impdata_create failed: " + std::to_string(ret));
            }

            ret = peconvolver->conv->start_process(CONVPROC_SCHEDULER_PRIORITY,
                                                   CONVPROC_SCHEDULER_CLASS);

            if (ret != 0) {
                failed = true;
                util::warning(peconvolver->log_tag +
                              "start_process failed: " + std::to_string(ret));
            }

            peconvolver->adapter = gst_adapter_new();

            peconvolver->ready = (failed) ? false : true;
        } else {
            util::warning(peconvolver->log_tag +
                          "we will just passthrough data.");

            peconvolver->ready = false;
        }
    }
}

static void gst_peconvolver_process(GstPeconvolver* peconvolver,
                                    GstBuffer* buffer) {
    if (peconvolver->ready) {
        GstMapInfo map;

        gst_buffer_map(buffer, &map, GST_MAP_READWRITE);

        guint num_samples = map.size / peconvolver->bpf;

        // deinterleave
        for (unsigned int n = 0; n < num_samples; n++) {
            peconvolver->conv->inpdata(0)[n] = ((float*)map.data)[2 * n];
            peconvolver->conv->inpdata(1)[n] = ((float*)map.data)[2 * n + 1];
        }

        int ret = peconvolver->conv->process(THREAD_SYNC_MODE);

        if (ret != 0) {
            util::warning(peconvolver->log_tag +
                          "IR: process failed: " + std::to_string(ret));
        }

        // interleave
        for (unsigned int n = 0; n < num_samples; n++) {
            ((float*)map.data)[2 * n] = peconvolver->conv->outdata(0)[n];
            ((float*)map.data)[2 * n + 1] = peconvolver->conv->outdata(1)[n];
        }

        gst_buffer_unmap(buffer, &map);
    }
}

static void gst_peconvolver_finish_convolver(GstPeconvolver* peconvolver) {
    if (peconvolver->ready) {
        peconvolver->ready = false;

        if (peconvolver->conv != nullptr) {
            if (peconvolver->conv->state() != Convproc::ST_STOP) {
                peconvolver->conv->stop_process();
            }

            peconvolver->conv->cleanup();

            delete peconvolver->conv;
        }

        if (peconvolver->kernel_L != nullptr) {
            delete[] peconvolver->kernel_L;
        }

        if (peconvolver->kernel_R != nullptr) {
            delete[] peconvolver->kernel_R;
        }

        if (peconvolver->adapter != nullptr) {
            gst_adapter_clear(peconvolver->adapter);

            g_object_unref(peconvolver->adapter);
        }
    }
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
