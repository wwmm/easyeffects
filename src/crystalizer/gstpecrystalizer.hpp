#ifndef _GST_PECRYSTALIZER_H_
#define _GST_PECRYSTALIZER_H_

#include <ebur128.h>
#include <gst/audio/gstaudiofilter.h>
#include <array>
#include <mutex>
#include "filter.hpp"

G_BEGIN_DECLS

#define GST_TYPE_PECRYSTALIZER (gst_pecrystalizer_get_type())
#define GST_PECRYSTALIZER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PECRYSTALIZER, GstPecrystalizer))
#define GST_PECRYSTALIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PECRYSTALIZER, GstPecrystalizerClass))
#define GST_IS_PECRYSTALIZER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PECRYSTALIZER))
#define GST_IS_PECRYSTALIZER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PECRYSTALIZER))

typedef struct _GstPecrystalizer GstPecrystalizer;
typedef struct _GstPecrystalizerClass GstPecrystalizerClass;

#define NBANDS 13

struct _GstPecrystalizer {
  GstAudioFilter base_pecrystalizer;

  /* properties */

  std::array<float, NBANDS - 1> freqs;
  std::array<float, NBANDS> intensities;
  std::array<bool, NBANDS> mute, bypass;

  float range_before, range_after;  // loudness range

  /* < private > */

  bool ready, notify, aggressive;
  int rate, bpf;  // sampling rate,  bytes per frame : channels * bps
  uint nsamples;
  int notify_samples;  // number of samples to count before emit a notify
  int sample_count, ndivs;
  float dv;

  std::array<Filter*, NBANDS> filters;
  std::array<std::vector<float>, NBANDS> band_data, gain;
  std::array<float, NBANDS> last_L, last_R, delayed_L, delayed_R;

  std::vector<float> deriv2;

  ebur128_state *ebur_state_before, *ebur_state_after;

  std::mutex mutex;

  GstPad *srcpad = nullptr, *sinkpad = nullptr;
};

struct _GstPecrystalizerClass {
  GstAudioFilterClass base_pecrystalizer_class;
};

GType gst_pecrystalizer_get_type(void);

G_END_DECLS

#endif
