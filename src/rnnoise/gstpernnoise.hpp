/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GST_PERNNOISE_HPP
#define GST_PERNNOISE_HPP

#include <gst/base/gstadapter.h>
#include <gst/gst.h>
#include <vector>

extern "C" {
#include "rnnoise.h"
}

G_BEGIN_DECLS

#define GST_TYPE_PERNNOISE (gst_pernnoise_get_type())
#define GST_PERNNOISE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PERNNOISE, GstPernnoise))
#define GST_PERNNOISE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PERNNOISE, GstPernnoiseClass))
#define GST_IS_PERNNOISE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PERNNOISE))
#define GST_IS_PERNNOISE_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PERNNOISE))

/**
 * GstPernnoise:
 *
 * The private pernnoise structure
 */
struct GstPernnoise {
  GstElement parent;

  /* properties */

  gchar* model_path = nullptr;
  gchar* model_name = nullptr;

  /*< private >*/

  int rate;              // sampling rate
  int bpf;               // bytes per frame : channels * bps
  int inbuf_n_samples;   // number of samples in the input buffer
  int outbuf_n_samples;  // number of samples in the input buffer
  int blocksize;         // number of samples processed by the rnnoise library
  bool flag_discont;
  bool ready;

  RNNModel* model = nullptr;
  DenoiseState *state_left = nullptr, *state_right = nullptr;

  std::vector<float> data_L;  // left channel buffer
  std::vector<float> data_R;  // right channel buffer

  GstAdapter* adapter = nullptr;
  GstAdapter* out_adapter = nullptr;
  GstPad* srcpad = nullptr;
  GstPad* sinkpad = nullptr;
};

struct GstPernnoiseClass {
  GstElementClass parent_class;
};

G_GNUC_INTERNAL GType gst_pearnnoise_get_type(void);

G_END_DECLS

#endif
