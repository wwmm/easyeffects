#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include <cmath>
#include "reverb.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Reverb* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "reverb_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Reverb*>(d);

                    if (success) {
                        util::debug(l->log_tag + "reverb enabled");
                    } else {
                        util::debug(l->log_tag + "failed to enable the reverb");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Reverb*>(d);

                    if (success) {
                        util::debug(l->log_tag + "reverb disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the reverb");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Reverb* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->input_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float inL, inR;

                g_object_get(l->reverb, "meter-inL", &inL, nullptr);
                g_object_get(l->reverb, "meter-inR", &inR, nullptr);

                std::array<double, 2> in_peak = {inL, inR};

                l->input_level.emit(in_peak);

                return true;
            },
            100);

        l->output_level_connection = Glib::signal_timeout().connect(
            [l]() {
                float outL, outR;

                g_object_get(l->reverb, "meter-outL", &outL, nullptr);
                g_object_get(l->reverb, "meter-outR", &outR, nullptr);

                std::array<double, 2> out_peak = {outL, outR};

                l->output_level.emit(out_peak);

                return true;
            },
            100);

    } else {
        l->input_level_connection.disconnect();
        l->output_level_connection.disconnect();
    }
}

}  // namespace

Reverb::Reverb(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    reverb = gst_element_factory_make("calf-sourceforge-net-plugins-Reverb",
                                      nullptr);

    plugin = gst_insert_bin_new("reverb_plugin");

    if (reverb != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Reverb plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("reverb_bin");

        gst_insert_bin_append(GST_INSERT_BIN(bin), reverb, nullptr, nullptr);

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

Reverb::~Reverb() {}

void Reverb::bind_to_gsettings() {
    g_settings_bind_with_mapping(
        settings, "input-gain", reverb, "level-in", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "output-gain", reverb, "level-out", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "room-size", reverb, "room-size",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "decay-time", reverb, "decay-time",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "hf-damp", reverb, "hf-damp",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "diffusion", reverb, "diffusion",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "amount", reverb, "amount", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "dry", reverb, "dry", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "predelay", reverb, "predelay",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "bass-cut", reverb, "bass-cut",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "treble-cut", reverb, "treble-cut",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);
}
