#include <glibmm/main.h>
#include "autogain.hpp"
#include "util.hpp"

namespace {

void on_m_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
    float m;

    g_object_get(a->autogain, "m", &m, nullptr);
}

}  // namespace

AutoGain::AutoGain(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "autogain", schema) {
    autogain = gst_element_factory_make("peautogain", nullptr);

    if (is_installed(autogain)) {
        auto input_gain = gst_element_factory_make("volume", nullptr);
        auto in_level =
            gst_element_factory_make("level", "autogain_input_level");
        auto output_gain = gst_element_factory_make("volume", nullptr);
        auto out_level =
            gst_element_factory_make("level", "autogain_output_level");
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert,
                         autogain, output_gain, out_level, nullptr);
        gst_element_link_many(input_gain, in_level, audioconvert, autogain,
                              output_gain, out_level, nullptr);

        auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
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
        g_settings_bind(settings, "post-messages", autogain, "notify-host",
                        G_SETTINGS_BIND_DEFAULT);

        g_signal_connect(autogain, "notify::m", G_CALLBACK(on_m_changed), this);

        g_settings_bind_with_mapping(
            settings, "input-gain", input_gain, "volume",
            G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear_double,
            util::linear_double_gain_to_db20, nullptr, nullptr);

        g_settings_bind_with_mapping(
            settings, "output-gain", output_gain, "volume",
            G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear_double,
            util::linear_double_gain_to_db20, nullptr, nullptr);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

AutoGain::~AutoGain() {
    util::debug(log_tag + name + " destroyed");
}

void AutoGain::bind_to_gsettings() {
    g_settings_bind_with_mapping(settings, "target", autogain, "target",
                                 G_SETTINGS_BIND_GET, util::double_to_float,
                                 nullptr, nullptr, nullptr);

    g_settings_bind(settings, "weight-m", autogain, "weight-m",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "weight-s", autogain, "weight-s",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "weight-i", autogain, "weight-i",
                    G_SETTINGS_BIND_DEFAULT);
}
