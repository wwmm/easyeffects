/* GStreamer
 * Copyright (C) 2018 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_PECONVOLVER_H_
#define _GST_PECONVOLVER_H_

#include <gst/audio/gstaudiofilter.h>

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
};

struct _GstPeconvolverClass {
    GstAudioFilterClass base_peconvolver_class;
};

GType gst_peconvolver_get_type(void);

G_END_DECLS

#endif
