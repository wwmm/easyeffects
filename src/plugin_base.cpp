#include <mutex>
#include "plugin_base.hpp"
#include "util.hpp"

extern std::mutex pipeline_mutex;

namespace {

void on_state_changed(GSettings* settings, gchar* key, PluginBase* l) {
  if (l->plugin_is_installed) {
    bool enable = g_settings_get_boolean(settings, key);

    if (enable) {
      l->enable();
    } else {
      l->disable();
    }
  } else {
    g_settings_set_boolean(settings, "installed", false);
  }
}

void on_enable(gpointer user_data) {
  auto l = static_cast<PluginBase*>(user_data);

  auto b = gst_bin_get_by_name(GST_BIN(l->plugin),
                               std::string(l->name + "_bin").c_str());

  if (!b) {
    gst_element_set_locked_state(l->plugin, true);

    gst_element_unlink(l->identity_in, l->identity_out);

    gst_bin_add(GST_BIN(l->plugin), l->bin);

    gst_element_set_state(l->bin, GST_STATE_NULL);

    gst_element_link_many(l->identity_in, l->bin, l->identity_out, nullptr);

    gst_element_sync_state_with_parent(l->bin);

    gst_element_set_locked_state(l->plugin, false);

    GstState state, pending;

    gst_element_get_state(l->bin, &state, &pending, 5 * GST_SECOND);

    util::debug(l->log_tag + l->name +
                " enabled: " + gst_element_state_get_name(state) + " -> " +
                gst_element_state_get_name(pending));

  } else {
    util::debug(l->log_tag + l->name + " is already enabled");
  }
}

void on_disable(gpointer user_data) {
  auto l = static_cast<PluginBase*>(user_data);

  auto b = gst_bin_get_by_name(GST_BIN(l->plugin),
                               std::string(l->name + "_bin").c_str());

  if (b) {
    gst_element_set_locked_state(l->plugin, true);

    gst_element_set_state(l->bin, GST_STATE_NULL);

    gst_element_unlink_many(l->identity_in, l->bin, l->identity_out, nullptr);

    gst_bin_remove(GST_BIN(l->plugin), l->bin);

    gst_element_link(l->identity_in, l->identity_out);

    gst_element_set_locked_state(l->plugin, false);

    GstState state, pending;

    gst_element_get_state(l->bin, &state, &pending, 5 * GST_SECOND);

    util::debug(l->log_tag + l->name +
                " disabled: " + gst_element_state_get_name(state) + " -> " +
                gst_element_state_get_name(pending));
  } else {
    util::debug(l->log_tag + l->name + " is already disabled");
  }
}

GstPadProbeReturn on_pad_blocked(GstPad* pad,
                                 GstPadProbeInfo* info,
                                 gpointer user_data) {
  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  auto l = static_cast<PluginBase*>(user_data);

  auto srcpad = gst_element_get_static_pad(l->bin, "src");

  gst_pad_add_probe(
      srcpad,
      static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BLOCK |
                                   GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM),
      [](auto pad, auto info, auto d) {
        if (GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS) {
          return GST_PAD_PROBE_PASS;
        }

        gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

        // std::lock_guard<std::mutex> lock(pipeline_mutex);

        on_disable(d);

        return GST_PAD_PROBE_DROP;
      },
      user_data, nullptr);

  auto sinkpad = gst_element_get_static_pad(l->bin, "sink");

  gst_pad_send_event(sinkpad, gst_event_new_eos());

  gst_object_unref(sinkpad);
  gst_object_unref(srcpad);

  return GST_PAD_PROBE_OK;
}

}  // namespace

PluginBase::PluginBase(const std::string& tag,
                       const std::string& plugin_name,
                       const std::string& schema)
    : log_tag(tag),
      name(plugin_name),
      changing_pipeline(false),
      settings(g_settings_new(schema.c_str())) {
  plugin = gst_bin_new(std::string(name + "_plugin").c_str());
  identity_in = gst_element_factory_make("identity", nullptr);
  identity_out = gst_element_factory_make("identity", nullptr);

  gst_bin_add_many(GST_BIN(plugin), identity_in, identity_out, nullptr);
  gst_element_link_many(identity_in, identity_out, nullptr);

  auto sinkpad = gst_element_get_static_pad(identity_in, "sink");
  auto srcpad = gst_element_get_static_pad(identity_out, "src");

  gst_element_add_pad(plugin, gst_ghost_pad_new("sink", sinkpad));
  gst_element_add_pad(plugin, gst_ghost_pad_new("src", srcpad));

  g_object_unref(sinkpad);
  g_object_unref(srcpad);

  bin = gst_bin_new((name + "_bin").c_str());
}

PluginBase::~PluginBase() {
  auto enable = g_settings_get_boolean(settings, "state");

  if (!enable) {
    gst_object_unref(bin);
  }

  g_object_unref(settings);
}

bool PluginBase::is_installed(GstElement* e) {
  if (e != nullptr) {
    plugin_is_installed = true;

    g_settings_set_boolean(settings, "installed", true);

    g_signal_connect(settings, "changed::state", G_CALLBACK(on_state_changed),
                     this);

    return true;
  } else {
    plugin_is_installed = false;

    g_settings_set_boolean(settings, "installed", false);

    util::warning(name + " plugin was not found!");

    return false;
  }
}

void PluginBase::enable() {
  auto srcpad = gst_element_get_static_pad(identity_in, "src");

  GstState state, pending;

  gst_element_get_state(bin, &state, &pending, 5 * GST_SECOND);

  if (state != GST_STATE_PLAYING) {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE,
                      [](auto pad, auto info, auto d) {
                        // std::lock_guard<std::mutex> lock(pipeline_mutex);

                        on_enable(d);

                        return GST_PAD_PROBE_REMOVE;
                      },
                      this, nullptr);
  } else {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                      [](auto pad, auto info, auto d) {
                        // std::lock_guard<std::mutex> lock(pipeline_mutex);

                        on_enable(d);

                        return GST_PAD_PROBE_REMOVE;
                      },
                      this, nullptr);
  }

  g_object_unref(srcpad);
}

void PluginBase::disable() {
  auto srcpad = gst_element_get_static_pad(identity_in, "src");

  GstState state, pending;

  gst_element_get_state(bin, &state, &pending, 5 * GST_SECOND);

  if (state != GST_STATE_PLAYING) {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_IDLE,
                      [](auto pad, auto info, auto d) {
                        // std::lock_guard<std::mutex> lock(pipeline_mutex);

                        on_disable(d);

                        return GST_PAD_PROBE_REMOVE;
                      },
                      this, nullptr);
  } else {
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                      on_pad_blocked, this, nullptr);
  }

  g_object_unref(srcpad);
}
