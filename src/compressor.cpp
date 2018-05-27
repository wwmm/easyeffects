#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "compressor.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Compressor* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "compressor_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Compressor*>(d);

                    if (success) {
                        util::debug(l->log_tag + "compressor enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the compressor");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Compressor*>(d);

                    if (success) {
                        util::debug(l->log_tag + "compressor disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the compressor");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Compressor* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->compression_connection = Glib::signal_timeout().connect(
            [l]() {
                float compression;

                g_object_get(l->compressor, "compression", &compression,
                             nullptr);

                l->compression.emit(compression);

                return true;
            },
            100);
    } else {
        l->compression_connection.disconnect();
    }
}

}  // namespace

Compressor::Compressor(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    compressor = gst_element_factory_make(
        "calf-sourceforge-net-plugins-Compressor", nullptr);

    plugin = gst_insert_bin_new("compressor_plugin");

    if (compressor != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Compressor plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("compressor_bin");

        auto in_level =
            gst_element_factory_make("level", "compressor_input_level");
        auto out_level =
            gst_element_factory_make("level", "compressor_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), compressor, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(compressor, "bypass", false, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);
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

Compressor::~Compressor() {}

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
