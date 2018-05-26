#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "deesser.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Deesser* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "deesser_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Deesser*>(d);

                    if (success) {
                        util::debug(l->log_tag + "deesser enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the deesser");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Deesser*>(d);

                    if (success) {
                        util::debug(l->log_tag + "deesser disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the deesser");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Deesser* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->compression_connection = Glib::signal_timeout().connect(
            [l]() {
                float compression;

                g_object_get(l->deesser, "compression", &compression, nullptr);

                l->compression.emit(compression);

                return true;
            },
            100);

        l->detected_connection = Glib::signal_timeout().connect(
            [l]() {
                float detected;

                g_object_get(l->deesser, "detected", &detected, nullptr);

                l->detected.emit(detected);

                return true;
            },
            100);
    } else {
        l->compression_connection.disconnect();
        l->detected_connection.disconnect();
    }
}

}  // namespace

Deesser::Deesser(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    deesser = gst_element_factory_make("calf-sourceforge-net-plugins-Deesser",
                                       nullptr);

    plugin = gst_insert_bin_new("deesser_plugin");

    if (deesser != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Deesser plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("deesser_bin");

        auto in_level =
            gst_element_factory_make("level", "deesser_input_level");
        auto out_level =
            gst_element_factory_make("level", "deesser_output_level");

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), deesser, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(deesser, "bypass", false, nullptr);

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

Deesser::~Deesser() {}

void Deesser::bind_to_gsettings() {
    g_settings_bind(settings, "detection", deesser, "detection",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "mode", deesser, "mode", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "ratio", deesser, "ratio",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "threshold", deesser, "threshold", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "laxity", deesser, "laxity",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(
        settings, "makeup", deesser, "makeup", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "f1-freq", deesser, "f1-freq",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "f2-freq", deesser, "f2-freq",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "f1-level", deesser, "f1-level", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "f2-level", deesser, "f2-level", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "f2-q", deesser, "f2-q",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "sc-listen", deesser, "sc-listen",
                    G_SETTINGS_BIND_DEFAULT);
}
