#ifndef _GST_PECRYSTALIZER_H_
#define _GST_PECRYSTALIZER_H_

#include <gst/audio/gstaudiofilter.h>
#include <mutex>
#include "filter.hpp"

G_BEGIN_DECLS

#define GST_TYPE_PECRYSTALIZER (gst_pecrystalizer_get_type())
#define GST_PECRYSTALIZER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PECRYSTALIZER, GstPecrystalizer))
#define GST_PECRYSTALIZER_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PECRYSTALIZER, \
                           GstPecrystalizerClass))
#define GST_IS_PECRYSTALIZER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PECRYSTALIZER))
#define GST_IS_PECRYSTALIZER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PECRYSTALIZER))

typedef struct _GstPecrystalizer GstPecrystalizer;
typedef struct _GstPecrystalizerClass GstPecrystalizerClass;

struct _GstPecrystalizer {
  GstAudioFilter base_pecrystalizer;

  /* properties */

  float intensity_band0, intensity_band1, intensity_band2, intensity_band3,
      freq1, freq2, freq3;

  /* < private > */

  bool ready;
  int rate, bpf;  // sampling rate,  bytes per frame : channels * bps
  uint nsamples;
  float last_L_band0, last_L_band1, last_L_band2, last_L_band3, last_R_band0,
      last_R_band1, last_R_band2, last_R_band3;
  float *data_band0 = nullptr, *data_band1 = nullptr, *data_band2 = nullptr,
        *data_band3 = nullptr;
  bool mute_band0, mute_band1, mute_band2, mute_band3;

  std::mutex mutex;

  Filter *band0 = nullptr, *band1 = nullptr, *band2 = nullptr, *band3 = nullptr;
};

struct _GstPecrystalizerClass {
  GstAudioFilterClass base_pecrystalizer_class;
};

GType gst_pecrystalizer_get_type(void);

G_END_DECLS

#endif
