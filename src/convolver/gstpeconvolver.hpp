#ifndef _GST_PECONVOLVER_H_
#define _GST_PECONVOLVER_H_

#include <gst/audio/gstaudiofilter.h>
#include <zita-convolver.h>
#include <iostream>
#include <mutex>

G_BEGIN_DECLS

#define GST_TYPE_PECONVOLVER (gst_peconvolver_get_type())
#define GST_PECONVOLVER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PECONVOLVER, GstPeconvolver))
#define GST_PECONVOLVER_CLASS(klass)                        \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PECONVOLVER, \
                             GstPeconvolverClass))
#define GST_IS_PECONVOLVER(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PECONVOLVER))
#define GST_IS_PECONVOLVER_CLASS(obj) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PECONVOLVER))

typedef struct _GstPeconvolver GstPeconvolver;
typedef struct _GstPeconvolverClass GstPeconvolverClass;

struct _GstPeconvolver {
    GstAudioFilter base_peconvolver;

    /* properties */

    gchar* kernel_path;
    unsigned int ir_width;

    /* < private > */

    bool ready;
    int rate, kernel_n_frames;
    int bpf;                     // bytes per frame : channels * bps
    float *kernel_L, *kernel_R;  // left and right channels buffers

    std::string log_tag;

    GstAdapter* adapter;

    Convproc* conv;

    std::mutex lock_guard_zita;
};

struct _GstPeconvolverClass {
    GstAudioFilterClass base_peconvolve_class;
};

GType gst_peconvolver_get_type(void);

G_END_DECLS

#endif
