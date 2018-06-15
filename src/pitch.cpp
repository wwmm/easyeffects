#include <glibmm/main.h>
#include "pitch.hpp"
#include "util.hpp"

Pitch::Pitch(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "pitch", schema) {
    pitch = gst_element_factory_make(
        "ladspa-ladspa-rubberband-so-rubberband-pitchshifter-stereo", "pitch");

    if (is_installed(pitch)) {
        auto in_level = gst_element_factory_make("level", "pitch_input_level");
        auto out_level =
            gst_element_factory_make("level", "pitch_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), in_level, audioconvert, pitch, out_level,
                         nullptr);
        gst_element_link_many(in_level, audioconvert, pitch, out_level,
                              nullptr);

        auto pad_sink = gst_element_get_static_pad(in_level, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        bind_to_gsettings();

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Pitch::~Pitch() {
    util::debug(log_tag + name + " destroyed");
}

void Pitch::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "cents", pitch, "cents",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "semitones", pitch, "semitones",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "octaves", pitch, "octaves",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "crispness", pitch, "crispness",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "formant-preserving", pitch, "formant-preserving",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "faster", pitch, "faster",
                    G_SETTINGS_BIND_DEFAULT);
}
