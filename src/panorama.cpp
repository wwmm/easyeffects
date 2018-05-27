#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "panorama.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Panorama* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "panorama_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Panorama*>(d);

                    if (success) {
                        util::debug(l->log_tag + "panorama enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the panorama");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Panorama*>(d);

                    if (success) {
                        util::debug(l->log_tag + "panorama disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the panorama");
                    }
                },
                l);
        }
    }
}

}  // namespace

Panorama::Panorama(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    panorama = gst_element_factory_make("audiopanorama", nullptr);

    plugin = gst_insert_bin_new("panorama_plugin");

    if (panorama != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Panorama plugin was not found!");
    }

    if (is_installed) {
        bin = gst_bin_new("panorama_bin");

        auto in_level =
            gst_element_factory_make("level", "panorama_input_level");
        auto out_level =
            gst_element_factory_make("level", "panorama_output_level");

        gst_bin_add_many(GST_BIN(bin), in_level, panorama, out_level, nullptr);
        gst_element_link_many(in_level, panorama, out_level, nullptr);

        auto pad_sink = gst_element_get_static_pad(in_level, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_object_set(panorama, "method", 0, nullptr);  // Psychoacoustic Panning

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

Panorama::~Panorama() {}

void Panorama::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "position", panorama, "panorama",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);
}
