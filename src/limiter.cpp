#include <gst/insertbin/gstinsertbin.h>
#include "limiter.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Limiter* l) {
    auto enable = g_settings_get_boolean(settings, "state");
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "limiter_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Limiter*>(d);

                    if (success) {
                        util::debug(l->log_tag + "limiter enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the limiter");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Limiter*>(d);

                    if (success) {
                        util::debug(l->log_tag + "limiter disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the limiter");
                    }
                },
                l);
        }
    }
}

void on_message(GObject* object, GParamSpec* pspec, Limiter* l) {
    util::debug(l->log_tag + "oi");
}

}  // namespace

Limiter::Limiter(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    limiter = gst_element_factory_make("calf-sourceforge-net-plugins-Limiter",
                                       nullptr);

    plugin = gst_insert_bin_new("limiter_plugin");

    if (limiter != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Limiter plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("limiter_bin");

        auto autovolume = gst_element_factory_make("level", "autovolume");

        gst_insert_bin_append(GST_INSERT_BIN(bin), limiter, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), autovolume, nullptr,
                              nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Limiter::~Limiter() {}

void Limiter::bind_to_gsettings() {
    g_settings_bind(settings, "oversampling", limiter, "oversampling",
                    G_SETTINGS_BIND_DEFAULT);

    // g_signal_connect(limiter, "notify::meter-inL", G_CALLBACK(on_message),
    //                  this);
}
