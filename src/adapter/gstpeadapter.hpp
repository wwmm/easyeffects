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

#ifndef GST_PEADAPTER_HPP
#define GST_PEADAPTER_HPP

#include <gst/audio/audio.h>
#include <gst/base/gstadapter.h>
#include <gst/gst.h>
#include "config.h"
#include "util.hpp"

G_BEGIN_DECLS

#define GST_TYPE_PEADAPTER (gst_peadapter_get_type())
#define GST_PEADAPTER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PEADAPTER, GstPeadapter))
#define GST_PEADAPTER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PEADAPTER, GstPeadapterClass))
#define GST_IS_PEADAPTER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PEADAPTER))
#define GST_IS_PEADAPTER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PEADAPTER))

/**
 * GstPeadapter:
 *
 * The private peadapter structure
 */
struct GstPeadapter {
  GstElement parent;

  /* properties */

  int blocksize;  // number of samples in the outout buffer

  /*< private >*/

  int rate;             // sampling rate
  int bpf;              // bytes per frame : channels * bps
  int inbuf_n_samples;  // number of samples in the input buffer
  bool flag_discont;
  bool passthrough_power_of_2;

  GstAdapter* adapter = nullptr;
  GstPad* srcpad = nullptr;
  GstPad* sinkpad = nullptr;
};

struct GstPeadapterClass {
  GstElementClass parent_class;
};

G_GNUC_INTERNAL GType gst_peadapter_get_type(void);

G_END_DECLS

#endif
