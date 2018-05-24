#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include <cmath>
#include "bass_enhancer.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, BassEnhancer* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "bass_enhancer_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<BassEnhancer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "bass_enhancer enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the bass_enhancer");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<BassEnhancer*>(d);

                    if (success) {
                        util::debug(l->log_tag + "bass_enhancer disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the bass_enhancer");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings,
                              gchar* key,
                              BassEnhancer* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->harmonics_connection = Glib::signal_timeout().connect(
            [l]() {
                float harmonics;

                g_object_get(l->bass_enhancer, "meter-drive", &harmonics,
                             nullptr);

                l->harmonics.emit(harmonics);

                return true;
            },
            100);
    } else {
        l->harmonics_connection.disconnect();
    }
}

}  // namespace

BassEnhancer::BassEnhancer(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    bass_enhancer = gst_element_factory_make(
        "calf-sourceforge-net-plugins-BassEnhancer", nullptr);

    plugin = gst_insert_bin_new("bass_enhancer_plugin");

    if (bass_enhancer != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("BassEnhancer plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("bass_enhancer_bin");

        auto in_level =
            gst_element_factory_make("level", "bass_enhancer_input_level");
        auto out_level =
            gst_element_factory_make("level", "bass_enhancer_output_level");

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), bass_enhancer, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(bass_enhancer, "bypass", false, nullptr);

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

BassEnhancer::~BassEnhancer() {}

void BassEnhancer::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "input-gain", bass_enhancer,
                                 "level-in", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", bass_enhancer,
                                 "level-out", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear,
                                 util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "amount", bass_enhancer, "amount", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "harmonics", bass_enhancer, "drive",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "scope", bass_enhancer, "freq",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "floor", bass_enhancer, "floor",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "blend", bass_enhancer, "blend",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "floor-active", bass_enhancer, "floor-active",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "listen", bass_enhancer, "listen",
                    G_SETTINGS_BIND_DEFAULT);
}
