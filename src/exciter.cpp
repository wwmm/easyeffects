#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "exciter.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Exciter* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "exciter_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Exciter*>(d);

                    if (success) {
                        util::debug(l->log_tag + "exciter enabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to enable the exciter");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Exciter*>(d);

                    if (success) {
                        util::debug(l->log_tag + "exciter disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the exciter");
                    }
                },
                l);
        }
    }
}

void on_post_messages_changed(GSettings* settings, gchar* key, Exciter* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        l->harmonics_connection = Glib::signal_timeout().connect(
            [l]() {
                float harmonics;

                g_object_get(l->exciter, "meter-drive", &harmonics, nullptr);

                l->harmonics.emit(harmonics);

                return true;
            },
            100);
    } else {
        l->harmonics_connection.disconnect();
    }
}

}  // namespace

Exciter::Exciter(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    exciter = gst_element_factory_make("calf-sourceforge-net-plugins-Exciter",
                                       nullptr);

    plugin = gst_insert_bin_new("exciter_plugin");

    if (exciter != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Exciter plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("exciter_bin");

        auto in_level =
            gst_element_factory_make("level", "exciter_input_level");
        auto out_level =
            gst_element_factory_make("level", "exciter_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), exciter, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        g_object_set(exciter, "bypass", false, nullptr);

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

Exciter::~Exciter() {}

void Exciter::bind_to_gsettings() {
    g_settings_bind_with_mapping(
        settings, "input-gain", exciter, "level-in", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "output-gain", exciter, "level-out", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "amount", exciter, "amount", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "harmonics", exciter, "drive",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "scope", exciter, "freq",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "ceil", exciter, "ceil",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "blend", exciter, "blend",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "ceil-active", exciter, "ceil-active",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "listen", exciter, "listen",
                    G_SETTINGS_BIND_DEFAULT);
}
