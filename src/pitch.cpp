#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "pitch.hpp"
#include "util.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Pitch* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "pitch_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Pitch*>(d);

                    if (success) {
                        util::debug(l->log_tag + "pitch enabled");
                    } else {
                        util::debug(l->log_tag + "failed to enable the pitch");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Pitch*>(d);

                    if (success) {
                        util::debug(l->log_tag + "pitch disabled");
                    } else {
                        util::debug(l->log_tag + "failed to disable the pitch");
                    }
                },
                l);
        }
    }
}

}  // namespace

Pitch::Pitch(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    pitch = gst_element_factory_make(
        "ladspa-ladspa-rubberband-so-rubberband-pitchshifter-stereo", "pitch");

    plugin = gst_insert_bin_new("pitch_plugin");

    if (pitch != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Pitch plugin was not found!");
    }

    if (is_installed) {
        bin = gst_insert_bin_new("pitch_bin");

        auto in_level = gst_element_factory_make("level", "pitch_input_level");
        auto out_level =
            gst_element_factory_make("level", "pitch_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert, nullptr,
                              nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), pitch, nullptr, nullptr);
        gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Pitch::~Pitch() {}

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
