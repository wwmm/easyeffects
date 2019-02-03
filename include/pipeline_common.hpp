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
    gst_element_unlink(l->plugins[l->plugins_order_old[n - 1]],
                       l->plugins[l->plugins_order_old[n]]);
  }

  gst_element_unlink(l->plugins[l->plugins_order_old.back()], l->identity_out);

  // linking elements using the new plugins order

  if (gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]])) {
    util::debug(l->log_tag + "linked identity_in to " + l->plugins_order[0]);
  } else {
    util::debug(l->log_tag + "failed to link identity_in to " +
                l->plugins_order[0]);
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
    util::debug(l->log_tag + "linked " + l->plugins_order.back() +
                " to identity_out");
  } else {
    util::debug(l->log_tag + "failed to link " + l->plugins_order.back() +
                " to identity_out");
  }

  for (auto& p : l->plugins) {
    if (gst_element_sync_state_with_parent(p.second)) {
      util::debug(l->log_tag + "synced " + p.first + " state with parent");
    } else {
      util::debug(l->log_tag + "failed to sync " + p.first +
                  " state with parent");
    }
  }
}

template <typename T>
GstPadProbeReturn on_pad_idle(GstPad* pad,
                              GstPadProbeInfo* info,
                              gpointer user_data) {
  auto l = static_cast<T>(user_data);

  std::lock_guard<std::mutex> lock(l->pipeline_mutex);

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
  } else if (!std::equal(l->plugins_order.begin(), l->plugins_order.end(),
                         l->plugins_order_old.begin())) {
    update = true;
  }

  if (update) {
    std::string list;

    for (auto name : l->plugins_order) {
      list += name + ",";
    }

    util::debug(l->log_tag + "new plugins order: [" + list + "]");

    update_effects_order<T>(user_data);
  }

  return GST_PAD_PROBE_REMOVE;
}

template <typename T>
void on_plugins_order_changed(GSettings* settings, gchar* key, T* l) {
  auto srcpad = gst_element_get_static_pad(l->identity_in, "src");

  gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE, on_pad_idle<T*>, l,
                    nullptr);

  g_object_unref(srcpad);
}

}  // namespace

#endif
