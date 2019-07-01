#ifndef PIPELINE_COMMON_HPP
#define PIPELINE_COMMON_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <mutex>
#include "util.hpp"

namespace {

template <typename T>
void update_effects_order(gpointer user_data) {
  auto l = static_cast<T>(user_data);

  // unlinking elements using old plugins order

  gst_element_unlink(l->identity_in, l->plugins[l->plugins_order_old[0]]);

  for (long unsigned int n = 1; n < l->plugins_order_old.size(); n++) {
    gst_element_unlink(l->plugins[l->plugins_order_old[n - 1]], l->plugins[l->plugins_order_old[n]]);
  }

  gst_element_unlink(l->plugins[l->plugins_order_old.back()], l->identity_out);

  // linking elements using the new plugins order

  if (gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]])) {
    util::debug(l->log_tag + "linked identity_in to " + l->plugins_order[0]);
  } else {
    util::debug(l->log_tag + "failed to link identity_in to " + l->plugins_order[0]);
  }

  for (long unsigned int n = 1; n < l->plugins_order.size(); n++) {
    auto p1_name = l->plugins_order[n - 1];
    auto p2_name = l->plugins_order[n];

    if (gst_element_link(l->plugins[p1_name], l->plugins[p2_name])) {
      util::debug(l->log_tag + "linked " + p1_name + " to " + p2_name);
    } else {
      util::debug(l->log_tag + "failed to link " + p1_name + " to " + p2_name);
    }
  }

  if (gst_element_link(l->plugins[l->plugins_order.back()], l->identity_out)) {
    util::debug(l->log_tag + "linked " + l->plugins_order.back() + " to identity_out");
  } else {
    util::debug(l->log_tag + "failed to link " + l->plugins_order.back() + " to identity_out");
  }
}

template <typename T>
bool check_update(gpointer user_data) {
  auto l = static_cast<T>(user_data);

  bool update = false;
  gchar* name;
  GVariantIter* iter;

  g_settings_get(l->child_settings, "plugins", "as", &iter);

  l->plugins_order_old = l->plugins_order;
  l->plugins_order.clear();

  while (g_variant_iter_next(iter, "s", &name)) {
    l->plugins_order.push_back(name);
    g_free(name);
  }

  g_variant_iter_free(iter);

  if (l->plugins_order.size() != l->plugins_order_old.size()) {
    update = true;
  } else if (!std::equal(l->plugins_order.begin(), l->plugins_order.end(), l->plugins_order_old.begin())) {
    update = true;
  }

  if (update) {
    std::string list;

    for (auto name : l->plugins_order) {
      list += name + ",";
    }

    util::debug(l->log_tag + "new plugins order: [" + list + "]");
  }

  return update;
}

template <typename T>
static GstPadProbeReturn event_probe_cb(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) {
  if (GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_CUSTOM_DOWNSTREAM) {
    return GST_PAD_PROBE_PASS;
  }

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  if (check_update<T>(user_data)) {
    update_effects_order<T>(user_data);
  }

  return GST_PAD_PROBE_DROP;
}

template <typename T>
GstPadProbeReturn on_pad_blocked(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) {
  auto l = static_cast<T>(user_data);

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  auto srcpad = gst_element_get_static_pad(l->identity_out, "src");

  gst_pad_add_probe(srcpad,
                    static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
                    event_probe_cb<T>, user_data, NULL);

  auto sinkpad = gst_element_get_static_pad(l->queue_src, "sink");

  GstStructure* s = gst_structure_new_empty("reorder_plugins");

  GstEvent* event = gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, s);

  gst_pad_send_event(sinkpad, event);

  gst_object_unref(sinkpad);
  gst_object_unref(srcpad);

  return GST_PAD_PROBE_OK;
}

template <typename T>
GstPadProbeReturn on_pad_idle(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) {
  if (check_update<T>(user_data)) {
    update_effects_order<T>(user_data);
  }

  return GST_PAD_PROBE_REMOVE;
}

template <typename T>
void on_plugins_order_changed(GSettings* settings, gchar* key, T* l) {
  auto srcpad = gst_element_get_static_pad(l->source, "src");

  GstState state, pending;

  gst_element_get_state(l->pipeline, &state, &pending, 0);

  if (state != GST_STATE_PLAYING) {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE, on_pad_idle<T*>, l, nullptr);
  } else {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, on_pad_blocked<T*>, l, nullptr);
  }

  g_object_unref(srcpad);
}

}  // namespace

#endif
