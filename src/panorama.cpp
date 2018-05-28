#include <glibmm/main.h>
#include "panorama.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Panorama* l) {
    auto enable = g_settings_get_boolean(settings, key);

    if (enable) {
        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "sink"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<Panorama*>(d);

                auto plugin =
                    gst_bin_get_by_name(GST_BIN(l->plugin), "panorama_bin");

                if (!plugin) {
                    gst_element_unlink(l->identity_in, l->identity_out);

                    gst_bin_add(GST_BIN(l->plugin), l->bin);

                    gst_element_sync_state_with_parent(l->identity_in);
                    gst_element_sync_state_with_parent(l->bin);
                    gst_element_sync_state_with_parent(l->identity_out);

                    gst_element_link_many(l->identity_in, l->bin,
                                          l->identity_out, nullptr);
                }

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    } else {
        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "sink"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<Panorama*>(d);

                auto plugin =
                    gst_bin_get_by_name(GST_BIN(l->plugin), "panorama_bin");

                if (plugin) {
                    gst_element_unlink_many(l->identity_in, l->bin,
                                            l->identity_out, nullptr);

                    gst_bin_remove(GST_BIN(l->plugin), l->bin);
                    gst_element_set_state(l->bin, GST_STATE_NULL);

                    gst_element_sync_state_with_parent(l->identity_in);
                    gst_element_sync_state_with_parent(l->identity_out);

                    gst_element_link(l->identity_in, l->identity_out);
                }

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    }
}

}  // namespace

Panorama::Panorama(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    panorama = gst_element_factory_make("audiopanorama", nullptr);

    plugin = gst_bin_new("panorama_plugin");
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
