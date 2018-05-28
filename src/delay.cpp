#include <glibmm/main.h>
#include "delay.hpp"
#include "util.hpp"

namespace {

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
    : PluginBase(tag, "delay", schema) {
    delay = gst_element_factory_make(
        "lsp-plug-in-plugins-lv2-comp-delay-x2-stereo", nullptr);

    if (is_installed(delay)) {
        bin = gst_bin_new("delay_bin");

        auto in_level = gst_element_factory_make("level", "delay_input_level");
        auto out_level =
            gst_element_factory_make("level", "delay_output_level");

        gst_bin_add_many(GST_BIN(bin), in_level, delay, out_level, nullptr);
        gst_element_link_many(in_level, delay, out_level, nullptr);

        auto pad_sink = gst_element_get_static_pad(in_level, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        g_object_set(delay, "bypass", false, nullptr);
        g_object_set(delay, "mode-l", 1, nullptr);  // Distance
        g_object_set(delay, "mode-r", 1, nullptr);  // Distance
        g_object_set(delay, "g-out", 1.0f, nullptr);
        g_object_set(delay, "dry-l", 0.0f, nullptr);
        g_object_set(delay, "wet-l", 1.0f, nullptr);
        g_object_set(delay, "dry-r", 0.0f, nullptr);
        g_object_set(delay, "wet-r", 1.0f, nullptr);

        bind_to_gsettings();

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
