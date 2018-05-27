#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "crossfeed.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Crossfeed* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "crossfeed_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Crossfeed*>(d);

                    if (success) {
                        util::debug(l->log_tag + "crossfeed enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the crossfeed");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Crossfeed*>(d);

                    if (success) {
                        util::debug(l->log_tag + "crossfeed disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the crossfeed");
                    }
                },
                l);
        }
    }
}

}  // namespace

Crossfeed::Crossfeed(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    crossfeed = gst_element_factory_make("bs2b", nullptr);

    plugin = gst_insert_bin_new("crossfeed_plugin");

    if (crossfeed != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Crossfeed plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("crossfeed_bin");

        auto in_level =
            gst_element_factory_make("level", "crossfeed_input_level");
        auto out_level =
            gst_element_factory_make("level", "crossfeed_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), crossfeed, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Crossfeed::~Crossfeed() {}

void Crossfeed::bind_to_gsettings() {
    g_settings_bind(settings, "fcut", crossfeed, "fcut",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "feed", crossfeed, "feed",
                                 G_SETTINGS_BIND_GET, util::double_x10_to_int,
                                 nullptr, nullptr, nullptr);
}
