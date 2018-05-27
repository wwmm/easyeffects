#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "gate.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Gate* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "gate_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Gate*>(d);

                    if (success) {
                        util::debug(l->log_tag + "gate enabled");
                    } else {
                        util::debug(l->log_tag + "failed to enable the gate");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Gate*>(d);

                    if (success) {
                        util::debug(l->log_tag + "gate disabled");
                    } else {
                        util::debug(l->log_tag + "failed to disable the gate");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Gate* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->gating_connection = Glib::signal_timeout().connect(
            [l]() {
                float gating;

                g_object_get(l->gate, "gating", &gating, nullptr);

                l->gating.emit(gating);

                return true;
            },
            100);
    } else {
        l->gating_connection.disconnect();
    }
}

}  // namespace

Gate::Gate(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    gate =
        gst_element_factory_make("calf-sourceforge-net-plugins-Gate", "gate");

    plugin = gst_insert_bin_new("gate_plugin");

    if (gate != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Gate plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("gate_bin");

        auto in_level = gst_element_factory_make("level", "gate_input_level");
        auto out_level = gst_element_factory_make("level", "gate_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), gate, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(gate, "bypass", false, nullptr);

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

Gate::~Gate() {}

void Gate::bind_to_gsettings() {
    g_settings_bind(settings, "detection", gate, "detection",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "stereo-link", gate, "stereo-link",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "range", gate, "range",
                                 G_SETTINGS_BIND_GET, util::db20_gain_to_linear,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "attack", gate, "attack",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "release", gate, "release",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "ratio", gate, "ratio",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "threshold", gate, "threshold", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "makeup", gate, "makeup", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);
}
