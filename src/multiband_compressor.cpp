#include <glibmm/main.h>
#include "multiband_compressor.hpp"
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings,
                              gchar* key,
                              MultibandCompressor* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        if (!l->input_level_connection.connected()) {
            l->input_level_connection = Glib::signal_timeout().connect(
                [l]() {
                    float inL, inR;

                    g_object_get(l->multiband_compressor, "meter-inL", &inL,
                                 nullptr);
                    g_object_get(l->multiband_compressor, "meter-inR", &inR,
                                 nullptr);

                    std::array<double, 2> in_peak = {inL, inR};

                    l->input_level.emit(in_peak);

                    return true;
                },
                100);
        }

        if (!l->output_level_connection.connected()) {
            l->output_level_connection = Glib::signal_timeout().connect(
                [l]() {
                    float outL, outR;

                    g_object_get(l->multiband_compressor, "meter-outL", &outL,
                                 nullptr);
                    g_object_get(l->multiband_compressor, "meter-outR", &outR,
                                 nullptr);

                    std::array<double, 2> out_peak = {outL, outR};

                    l->output_level.emit(out_peak);

                    return true;
                },
                100);
        }

        if (!l->compression0_connection.connected()) {
            l->compression0_connection = Glib::signal_timeout().connect(
                [l]() {
                    float compression;

                    g_object_get(l->multiband_compressor, "compression0",
                                 &compression, nullptr);

                    l->compression0.emit(compression);

                    return true;
                },
                100);
        }

        if (!l->compression1_connection.connected()) {
            l->compression1_connection = Glib::signal_timeout().connect(
                [l]() {
                    float compression;

                    g_object_get(l->multiband_compressor, "compression1",
                                 &compression, nullptr);

                    l->compression1.emit(compression);

                    return true;
                },
                100);
        }

        if (!l->compression2_connection.connected()) {
            l->compression2_connection = Glib::signal_timeout().connect(
                [l]() {
                    float compression;

                    g_object_get(l->multiband_compressor, "compression2",
                                 &compression, nullptr);

                    l->compression2.emit(compression);

                    return true;
                },
                100);
        }

        if (!l->compression3_connection.connected()) {
            l->compression3_connection = Glib::signal_timeout().connect(
                [l]() {
                    float compression;

                    g_object_get(l->multiband_compressor, "compression3",
                                 &compression, nullptr);

                    l->compression3.emit(compression);

                    return true;
                },
                100);
        }
    } else {
        l->compression0_connection.disconnect();
        l->compression1_connection.disconnect();
        l->compression2_connection.disconnect();
        l->compression3_connection.disconnect();
    }
}

}  // namespace

MultibandCompressor::MultibandCompressor(const std::string& tag,
                                         const std::string& schema)
    : PluginBase(tag, "multiband_compressor", schema) {
    multiband_compressor = gst_element_factory_make(
        "calf-sourceforge-net-plugins-MultibandCompressor", nullptr);

    if (is_installed(multiband_compressor)) {
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), audioconvert, multiband_compressor,
                         nullptr);
        gst_element_link_many(audioconvert, multiband_compressor, nullptr);

        auto pad_sink = gst_element_get_static_pad(audioconvert, "sink");
        auto pad_src = gst_element_get_static_pad(multiband_compressor, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_object_set(multiband_compressor, "bypass", false, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::post-messages",
                         G_CALLBACK(on_post_messages_changed), this);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

MultibandCompressor::~MultibandCompressor() {
    util::debug(log_tag + name + " destroyed");
}

void MultibandCompressor::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "input-gain", multiband_compressor,
                                 "level-in", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", multiband_compressor,
                                 "level-out", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "freq0", multiband_compressor, "freq0", G_SETTINGS_BIND_GET,
        util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "freq1", multiband_compressor, "freq1", G_SETTINGS_BIND_GET,
        util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "freq2", multiband_compressor, "freq2", G_SETTINGS_BIND_GET,
        util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind(settings, "mode", multiband_compressor, "mode",
                    G_SETTINGS_BIND_DEFAULT);

    // sub band

    g_settings_bind_with_mapping(settings, "threshold0", multiband_compressor,
                                 "threshold0", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "ratio0", multiband_compressor, "ratio0", G_SETTINGS_BIND_GET,
        util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "attack0", multiband_compressor, "attack0",
        G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "release0", multiband_compressor, "release0",
        G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "makeup", multiband_compressor,
                                 "makeup", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "knee0", multiband_compressor,
                                 "knee0", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "detection0", multiband_compressor, "detection0",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "bypass0", multiband_compressor, "bypass0",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "solo0", multiband_compressor, "solo0",
                    G_SETTINGS_BIND_DEFAULT);
}
