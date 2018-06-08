#include <glibmm/main.h>
#include "bass_enhancer.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, PluginBase* l) {
    bool enable = g_settings_get_boolean(settings, key);

    if (enable) {
        l->enable();
    } else {
        l->disable();
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

BassEnhancer::BassEnhancer(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "bass_enhancer", schema) {
    bass_enhancer = gst_element_factory_make(
        "calf-sourceforge-net-plugins-BassEnhancer", nullptr);

    if (is_installed(bass_enhancer)) {
        bin = gst_bin_new("bass_enhancer_bin");

        auto in_level =
            gst_element_factory_make("level", "bass_enhancer_input_level");
        auto out_level =
            gst_element_factory_make("level", "bass_enhancer_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), in_level, audioconvert, bass_enhancer,
                         out_level, nullptr);
        gst_element_link_many(in_level, audioconvert, bass_enhancer, out_level,
                              nullptr);

        auto pad_sink = gst_element_get_static_pad(in_level, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

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
