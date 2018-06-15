#include <glibmm/main.h>
#include "expander.hpp"
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Expander* l) {
    auto post = g_settings_get_boolean(settings, key);

    if (post) {
        if (!l->input_level_connection.connected()) {
            l->input_level_connection = Glib::signal_timeout().connect(
                [l]() {
                    float inL, inR;

                    g_object_get(l->expander, "ilm-l", &inL, nullptr);
                    g_object_get(l->expander, "ilm-r", &inR, nullptr);

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

                    g_object_get(l->expander, "olm-l", &outL, nullptr);
                    g_object_get(l->expander, "olm-r", &outR, nullptr);

                    std::array<double, 2> out_peak = {outL, outR};

                    l->output_level.emit(out_peak);

                    return true;
                },
                100);
        }
    } else {
        l->input_level_connection.disconnect();
        l->output_level_connection.disconnect();
    }
}

}  // namespace

Expander::Expander(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "expander", schema) {
    expander = gst_element_factory_make(
        "lsp-plug-in-plugins-lv2-expander-stereo", nullptr);

    if (is_installed(expander)) {
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), audioconvert, expander, nullptr);
        gst_element_link_many(audioconvert, expander, nullptr);

        auto pad_sink = gst_element_get_static_pad(audioconvert, "sink");
        auto pad_src = gst_element_get_static_pad(expander, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_object_set(expander, "bypass", false, nullptr);
        g_object_set(expander, "pause", true, nullptr);  // Pause graph analysis
        g_object_set(expander, "g-in", 1.0f, nullptr);
        g_object_set(expander, "g-out", 1.0f, nullptr);
        g_object_set(expander, "cdr", 0.0f, nullptr);
        g_object_set(expander, "cwt", 1.0f, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::post-messages",
                         G_CALLBACK(on_post_messages_changed), this);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Expander::~Expander() {
    util::debug(log_tag + name + " destroyed");
}

void Expander::bind_to_gsettings() {
    g_settings_bind(settings, "scm", expander, "scm", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "sla", expander, "sla",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "scl", expander, "scl", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "scs", expander, "scs", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "scr", expander, "scr",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "scp", expander, "scp", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "em", expander, "em", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(
        settings, "al", expander, "al", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "at", expander, "at",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "rrl", expander, "rrl", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "rt", expander, "rt",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "cr", expander, "cr",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "kn", expander, "kn", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "mk", expander, "mk", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);
}
