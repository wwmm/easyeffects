#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "delay.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Delay* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "delay_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Delay*>(d);

                    if (success) {
                        util::debug(l->log_tag + "delay enabled");
                    } else {
                        util::debug(l->log_tag + "failed to enable the delay");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Delay*>(d);

                    if (success) {
                        util::debug(l->log_tag + "delay disabled");
                    } else {
                        util::debug(l->log_tag + "failed to disable the delay");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Delay* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->tempo_connection = Glib::signal_timeout().connect_seconds(
            [l]() {
                float L, R;

                g_object_get(l->delay, "d-t-l", &L, nullptr);
                g_object_get(l->delay, "d-t-r", &R, nullptr);

                std::array<double, 2> tempo = {L, R};

                l->tempo.emit(tempo);

                return true;
            },
            1);
    } else {
        l->tempo_connection.disconnect();
    }
}

}  // namespace

Delay::Delay(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    delay = gst_element_factory_make(
        "lsp-plug-in-plugins-lv2-comp-delay-x2-stereo", nullptr);

    plugin = gst_insert_bin_new("delay_plugin");

    if (delay != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Delay plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("delay_bin");

        auto in_level = gst_element_factory_make("level", "delay_input_level");
        auto out_level =
            gst_element_factory_make("level", "delay_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), delay, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(delay, "bypass", false, nullptr);
        g_object_set(delay, "mode-l", 1, nullptr);  // Distance
        g_object_set(delay, "mode-r", 1, nullptr);  // Distance
        g_object_set(delay, "g-out", 1.0f, nullptr);
        g_object_set(delay, "dry-l", 0.0f, nullptr);
        g_object_set(delay, "wet-l", 1.0f, nullptr);
        g_object_set(delay, "dry-r", 0.0f, nullptr);
        g_object_set(delay, "wet-r", 1.0f, nullptr);

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

Delay::~Delay() {}

void Delay::bind_to_gsettings() {
    g_settings_bind(settings, "m-l", delay, "m-l", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "m-r", delay, "m-r", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "cm-l", delay, "cm-l",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "cm-r", delay, "cm-r",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "temperature", delay, "t-l",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "temperature", delay, "t-r",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);
}
