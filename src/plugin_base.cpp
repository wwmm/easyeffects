#include "plugin_base.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, PluginBase* l) {
    bool enable = g_settings_get_boolean(settings, key);

    if (enable) {
        l->enable();
    } else {
        l->disable();
    }
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
}

PluginBase::~PluginBase() {
    g_object_unref(settings);
}

bool PluginBase::is_installed(GstElement* e) {
    if (e != nullptr) {
        g_settings_set_boolean(settings, "installed", true);

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);

        return true;
    } else {
        g_settings_set_boolean(settings, "installed", false);

        util::warning(name + " plugin was not found!");

        return false;
    }
}

void PluginBase::enable() {
    auto b = gst_bin_get_by_name(GST_BIN(plugin),
                                 std::string(name + "_bin").c_str());

    if (!b) {
        bool changing = changing_pipeline.exchange(true);

        if (!changing) {
            gst_pad_add_probe(
                gst_element_get_static_pad(identity_in, "src"),
                GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                [](auto pad, auto info, auto d) {
                    auto l = static_cast<PluginBase*>(d);

                    gst_element_unlink(l->identity_in, l->identity_out);

                    gst_bin_add(GST_BIN(l->plugin), l->bin);

                    gst_element_link_many(l->identity_in, l->bin,
                                          l->identity_out, nullptr);

                    gst_bin_sync_children_states(GST_BIN(l->plugin));

                    util::debug(l->log_tag + l->name + " enabled");

                    l->changing_pipeline = false;

                    return GST_PAD_PROBE_REMOVE;
                },
                this, nullptr);
        }
    }
}

void PluginBase::disable() {
    auto b = gst_bin_get_by_name(GST_BIN(plugin),
                                 std::string(name + "_bin").c_str());

    if (b) {
        bool changing = changing_pipeline.exchange(true);

        if (!changing) {
            gst_pad_add_probe(
                gst_element_get_static_pad(identity_in, "src"),
                GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                [](auto pad, auto info, auto d) {
                    auto l = static_cast<PluginBase*>(d);

                    gst_element_unlink_many(l->identity_in, l->bin,
                                            l->identity_out, nullptr);

                    gst_bin_remove(GST_BIN(l->plugin), l->bin);

                    gst_element_set_state(l->bin, GST_STATE_NULL);

                    gst_element_link(l->identity_in, l->identity_out);

                    gst_bin_sync_children_states(GST_BIN(l->plugin));

                    util::debug(l->log_tag + l->name + " disabled");

                    l->changing_pipeline = false;

                    return GST_PAD_PROBE_REMOVE;
                },
                this, nullptr);
        }
    }
}
