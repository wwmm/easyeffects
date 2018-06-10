#include <glibmm/main.h>
#include "panorama.hpp"
#include "util.hpp"

Panorama::Panorama(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "panorama", schema) {
    panorama = gst_element_factory_make("audiopanorama", nullptr);

    if (is_installed(panorama)) {
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

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force PluginBase on_state_changed callback call

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
