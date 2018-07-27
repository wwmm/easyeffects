#ifndef _GST_PEAUTOGAIN_H_
#define _GST_PEAUTOGAIN_H_

#include <ebur128.h>
#include <gst/audio/gstaudiofilter.h>
#include <mutex>

G_BEGIN_DECLS

#define GST_TYPE_PEAUTOGAIN (gst_peautogain_get_type())
#define GST_PEAUTOGAIN(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PEAUTOGAIN, GstPeautogain))
#define GST_PEAUTOGAIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PEAUTOGAIN, GstPeautogainClass))
#define GST_IS_PEAUTOGAIN(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PEAUTOGAIN))
#define GST_IS_PEAUTOGAIN_CLASS(obj) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PEAUTOGAIN))

typedef struct _GstPeautogain GstPeautogain;
typedef struct _GstPeautogainClass GstPeautogainClass;

struct _GstPeautogain {
    GstAudioFilter base_peautogain;

    /* properties */

    float target;  // target loudness level
    int weight_m;  // momentary loudness weight
    int weight_s;  // short term loudness weight
    int weight_i;  // integrated loudness weight

    /* < private > */

    bool ready;
    int bpf;   // bytes per frame : channels * bps
    int rate;  // sampling rate
    float gain;
    ebur128_state* ebur_state;

    std::mutex lock_guard_ebu;
};

struct _GstPeautogainClass {
    GstAudioFilterClass base_peautogain_class;
};

GType gst_peautogain_get_type(void);

G_END_DECLS

#endif
