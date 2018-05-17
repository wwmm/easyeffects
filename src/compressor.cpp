#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include <cmath>
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
        l->input_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float inL, inR;

                g_object_get(l->compressor, "meter-inL", &inL, nullptr);
                g_object_get(l->compressor, "meter-inR", &inR, nullptr);

                std::array<double, 2> in_peak = {inL, inR};

                l->input_level.emit(in_peak);

                return true;
            },
            100);

        l->output_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float outL, outR;

                g_object_get(l->compressor, "meter-outL", &outL, nullptr);
                g_object_get(l->compressor, "meter-outR", &outR, nullptr);

                std::array<double, 2> out_peak = {outL, outR};

                l->output_level.emit(out_peak);

                return true;
            },
            100);

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
        l->input_level_connection.disconnect();
        l->output_level_connection.disconnect();
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

        gst_insert_bin_append(GST_INSERT_BIN(bin), compressor, nullptr,
                              nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);
        g_signal_connect(settings, "changed::post-messages",
                         G_CALLBACK(on_post_messages_changed), this);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Compressor::~Compressor() {}

void Compressor::bind_to_gsettings() {
    g_settings_bind_with_mapping(
        settings, "input-gain", compressor, "level-in", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "limit", compressor, "limit",
                                 G_SETTINGS_BIND_GET, util::db20_gain_to_linear,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "lookahead", compressor, "attack",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "release", compressor, "release",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "asc", compressor, "asc",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "asc-level", compressor, "asc-coeff",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "oversampling", compressor, "oversampling",
                    G_SETTINGS_BIND_DEFAULT);
}

double Compressor::get_compression() {
    float compression;

    g_object_get(compressor, "att", &compression, nullptr);

    return compression;
}
