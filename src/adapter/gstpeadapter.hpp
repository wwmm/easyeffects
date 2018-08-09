#ifndef __GST_PEADAPTER_H__
#define __GST_PEADAPTER_H__

#include <gst/gst.h>

G_BEGIN_DECLS
/* #define's don't like whitespacey bits */
#define GST_TYPE_PEADAPTER (gst_peadapter_get_type())
#define GST_PEADAPTER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_PEADAPTER, GstPeadapter))
#define GST_PEADAPTER_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_PEADAPTER, GstPeadapterClass))
#define GST_IS_PEADAPTER(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_PEADAPTER))
#define GST_IS_PEADAPTER_CLASS(obj) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_PEADAPTER))

typedef struct _GstPeadapter GstPeadapter;
typedef struct _GstPeadapterClass GstPeadapterClass;

/**
 * GstPeadapter:
 *
 * The private peadapter structure
 */
struct _GstPeadapter {
    /*< private >*/
    GstElement parent;

    /* atomic boolean */
    volatile gint drop;

    /* Protected by the stream lock */
    gboolean discont;
    gboolean need_repush_sticky;

    GstPad* srcpad;
    GstPad* sinkpad;
};

struct _GstPeadapterClass {
    GstElementClass parent_class;
};

G_GNUC_INTERNAL GType gst_peadapter_get_type(void);

G_END_DECLS

#endif /* __GST_PEADAPTER_H__ */
