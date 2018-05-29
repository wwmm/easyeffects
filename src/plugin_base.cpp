#include "plugin_base.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, PluginBase* l) {
    auto enable = g_settings_get_boolean(settings, key);

    if (enable) {
        while (l->in_pad_cb) {
        }

        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "src"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<PluginBase*>(d);

                l->in_pad_cb = true;

                auto plugin = gst_bin_get_by_name(
                    GST_BIN(l->plugin), std::string(l->name + "_bin").c_str());

                if (!plugin) {
                    gst_element_unlink(l->identity_in, l->identity_out);

                    gst_bin_add(GST_BIN(l->plugin), l->bin);

                    gst_bin_sync_children_states(GST_BIN(l->plugin));

                    gst_element_link_many(l->identity_in, l->bin,
                                          l->identity_out, nullptr);

                    util::debug(l->log_tag + l->name + " enabled");
                }

                l->in_pad_cb = false;

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);

    } else {
        while (l->in_pad_cb) {
        }

        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "src"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<PluginBase*>(d);

                l->in_pad_cb = true;

                auto plugin = gst_bin_get_by_name(
                    GST_BIN(l->plugin), std::string(l->name + "_bin").c_str());

                if (plugin) {
                    gst_element_unlink_many(l->identity_in, l->bin,
                                            l->identity_out, nullptr);

                    gst_bin_remove(GST_BIN(l->plugin), l->bin);

                    gst_element_set_state(l->bin, GST_STATE_NULL);

                    gst_bin_sync_children_states(GST_BIN(l->plugin));

                    gst_element_link(l->identity_in, l->identity_out);

                    util::debug(l->log_tag + l->name + " disabled");
                }

                l->in_pad_cb = false;

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    }
}

}  // namespace

PluginBase::PluginBase(std::string tag,
                       std::string plugin_name,
                       std::string schema)
    : log_tag(tag),
      name(plugin_name),
      settings(g_settings_new(schema.c_str())) {
    plugin = gst_bin_new(std::string(name + "_plugin").c_str());
    identity_in = gst_element_factory_make("identity", nullptr);
    identity_out = gst_element_factory_make("identity", nullptr);

    gst_bin_add_many(GST_BIN(plugin), identity_in, identity_out, nullptr);
    gst_element_link_many(identity_in, identity_out, nullptr);

    gst_element_add_pad(
        plugin, gst_ghost_pad_new(
                    "sink", gst_element_get_static_pad(identity_in, "sink")));
    gst_element_add_pad(
        plugin, gst_ghost_pad_new(
                    "src", gst_element_get_static_pad(identity_out, "src")));

    g_signal_connect(settings, "changed::state", G_CALLBACK(on_state_changed),
                     this);
}

PluginBase::~PluginBase() {}

bool PluginBase::is_installed(GstElement* e) {
    if (e != nullptr) {
        return true;
    } else {
        util::warning(name + " plugin was not found!");

        return false;
    }
}
