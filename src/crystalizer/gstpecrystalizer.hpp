#ifndef _GST_PECRYSTALIZER_H_
#define _GST_PECRYSTALIZER_H_

#include <gst/audio/gstaudiofilter.h>
#include <array>
#include <future>
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

#define NBANDS 13

struct _GstPecrystalizer {
  GstAudioFilter base_pecrystalizer;

  /* properties */

  std::array<float, NBANDS - 1> freqs;
  std::array<float, NBANDS> intensities;
  std::array<bool, NBANDS> mute, bypass;

  /* < private > */

  bool ready;
  int rate, bpf;  // sampling rate,  bytes per frame : channels * bps
  uint nsamples;

  std::array<Filter*, NBANDS> filters;
  std::array<std::vector<float>, NBANDS> band_data;
  std::array<float, NBANDS> last_L, last_R;

  std::vector<float> tmp_band_data;

  std::mutex mutex;

  std::vector<std::future<void>> futures;
};

struct _GstPecrystalizerClass {
  GstAudioFilterClass base_pecrystalizer_class;
};

GType gst_pecrystalizer_get_type(void);

G_END_DECLS

#endif
