#ifndef __GST_PEADAPTER_H__
#define __GST_PEADAPTER_H__

#include <gst/base/gstadapter.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_PEADAPTER (gst_peadapter_get_type())
#define GST_PEADAPTER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PEADAPTER, GstPeadapter))
#define GST_PEADAPTER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PEADAPTER, GstPeadapterClass))
#define GST_IS_PEADAPTER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PEADAPTER))
#define GST_IS_PEADAPTER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PEADAPTER))

typedef struct _GstPeadapter GstPeadapter;
typedef struct _GstPeadapterClass GstPeadapterClass;

/**
 * GstPeadapter:
 *
 * The private peadapter structure
 */
struct _GstPeadapter {
  GstElement parent;

  /* properties */

  int blocksize;  // number of samples in the outout buffer

  /*< private >*/

  int rate;             // sampling rate
  int bpf;              // bytes per frame : channels * bps
  int inbuf_n_samples;  // number of samples in the input buffer
  bool flag_discont;

  GstAdapter* adapter = nullptr;
  GstPad* srcpad = nullptr;
  GstPad* sinkpad = nullptr;
};

struct _GstPeadapterClass {
  GstElementClass parent_class;
};

G_GNUC_INTERNAL GType gst_peadapter_get_type(void);

G_END_DECLS

#endif /* __GST_PEADAPTER_H__ */
