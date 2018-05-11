#include <gst/insertbin/gstinsertbin.h>
#include "limiter.hpp"
#include "util.hpp"

Limiter::Limiter(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    limiter = gst_element_factory_make("calf-sourceforge-net-plugins-Limiter",
                                       nullptr);

    if (limiter != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Limiter plugin was not found. Disabling it!");
    }
}

Limiter::~Limiter() {}
