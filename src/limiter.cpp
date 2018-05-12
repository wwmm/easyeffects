#include <gst/insertbin/gstinsertbin.h>
#include "limiter.hpp"
#include "util.hpp"

Limiter::Limiter(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    limiter = gst_element_factory_make("calf-sourceforge-net-plugins-Limiter",
                                       nullptr);

    if (limiter != nullptr) {
        is_installed = true;

        enable = g_settings_get_boolean(settings, "state");
    } else {
        is_installed = false;
        enable = false;

        util::warning("Limiter plugin was not found. Disabling it!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("limiter_bin");

        auto autovolume = gst_element_factory_make("level", "autovolume");

        gst_insert_bin_append(GST_INSERT_BIN(bin), limiter, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), autovolume, nullptr,
                              nullptr);
    }
}

Limiter::~Limiter() {}
