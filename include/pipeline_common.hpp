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

#ifndef PIPELINE_COMMON_HPP
#define PIPELINE_COMMON_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <mutex>
#include "util.hpp"

template <typename T>
void update_effects_order(gpointer user_data) {
  auto l = static_cast<T>(user_data);

  // unlinking elements using old plugins order

  gst_element_unlink(l->identity_in, l->plugins[l->plugins_order_old[0]]);

  for (unsigned long int n = 1U; n < l->plugins_order_old.size(); n++) {
    gst_element_unlink(l->plugins[l->plugins_order_old[n - 1U]], l->plugins[l->plugins_order_old[n]]);
  }

  gst_element_unlink(l->plugins[l->plugins_order_old.back()], l->identity_out);

  // linking elements using the new plugins order

  if (gst_element_link(l->identity_in, l->plugins[l->plugins_order[0]])) {
    util::debug(l->log_tag + "linked identity_in to " + l->plugins_order[0]);
  } else {
    util::debug(l->log_tag + "failed to link identity_in to " + l->plugins_order[0]);
  }

  for (unsigned long int n = 1U; n < l->plugins_order.size(); n++) {
    auto p1_name = l->plugins_order[n - 1U];
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
auto check_update(gpointer user_data) -> bool {
  auto l = static_cast<T>(user_data);

  bool update = false;
  gchar* name = nullptr;
  GVariantIter* iter = nullptr;

  g_settings_get(l->child_settings, "plugins", "as", &iter);

  l->plugins_order_old = l->plugins_order;
  l->plugins_order.clear();

  while (g_variant_iter_next(iter, "s", &name)) {
    l->plugins_order.emplace_back(name);
    g_free(name);
  }

  g_variant_iter_free(iter);

  if (l->plugins_order.size() != l->plugins_order_old.size()) {
    update = true;
  }

  if (!std::equal(l->plugins_order.begin(), l->plugins_order.end(), l->plugins_order_old.begin())) {
    update = true;
  }

  if (update) {
    std::string list;

    for (const auto& name : l->plugins_order) {
      list += name + ",";
    }

    util::debug(l->log_tag + "new plugins order: [" + list + "]");
  }

  return update;
}

template <typename T>
auto event_probe_cb(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
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
auto on_pad_blocked(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
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
auto on_pad_idle(GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
  if (check_update<T>(user_data)) {
    update_effects_order<T>(user_data);
  }

  return GST_PAD_PROBE_REMOVE;
}

template <typename T>
void on_plugins_order_changed(GSettings* settings, gchar* key, T* l) {
  auto srcpad = gst_element_get_static_pad(l->source, "src");

  GstState state;
  GstState pending;

  gst_element_get_state(l->pipeline, &state, &pending, 0);

  if (state != GST_STATE_PLAYING) {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE, on_pad_idle<T*>, l, nullptr);
  } else {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, on_pad_blocked<T*>, l, nullptr);
  }

  g_object_unref(srcpad);
}

#endif
