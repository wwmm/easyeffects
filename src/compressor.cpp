#include <glibmm/main.h>
#include "compressor.hpp"
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Compressor* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        if (!l->compression_connection.connected()) {
            l->compression_connection = Glib::signal_timeout().connect(
                [l]() {
                    float compression;

                    g_object_get(l->compressor, "compression", &compression,
                                 nullptr);

                    l->compression.emit(compression);

                    return true;
                },
                100);
        }
    } else {
        l->compression_connection.disconnect();
    }
}

}  // namespace

Compressor::Compressor(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "compressor", schema) {
    compressor = gst_element_factory_make(
        "calf-sourceforge-net-plugins-Compressor", nullptr);

    if (is_installed(compressor)) {
        auto in_level =
            gst_element_factory_make("level", "compressor_input_level");
        auto out_level =
            gst_element_factory_make("level", "compressor_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), in_level, audioconvert, compressor,
                         out_level, nullptr);
        gst_element_link_many(in_level, audioconvert, compressor, out_level,
                              nullptr);

        auto pad_sink = gst_element_get_static_pad(in_level, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_object_set(compressor, "bypass", false, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::post-messages",
                         G_CALLBACK(on_post_messages_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Compressor::~Compressor() {
    util::debug(log_tag + name + " destroyed");
}

void Compressor::bind_to_gsettings() {
    g_settings_bind(settings, "detection", compressor, "detection",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "stereo-link", compressor, "stereo-link",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "mix", compressor, "mix",
                                 G_SETTINGS_BIND_GET, util::db20_gain_to_linear,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "attack", compressor, "attack",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "release", compressor, "release",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "ratio", compressor, "ratio",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "threshold", compressor, "threshold", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "makeup", compressor, "makeup", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);
}
