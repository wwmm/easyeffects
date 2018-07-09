#include <glibmm/main.h>
#include <gst/fft/gstfftf32.h>
#include "convolver.hpp"
#include "util.hpp"

namespace {}  // namespace

Convolver::Convolver(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "convolver", schema) {
    convolver = gst_element_factory_make("peconvolver", "convolver");

    if (is_installed(convolver)) {
        auto input_gain = gst_element_factory_make("volume", nullptr);
        auto in_level =
            gst_element_factory_make("level", "convolver_input_level");
        auto out_level =
            gst_element_factory_make("level", "convolver_output_level");
        auto output_gain = gst_element_factory_make("volume", nullptr);
        auto audioconvert = gst_element_factory_make("audioconvert", nullptr);

        gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert,
                         convolver, output_gain, out_level, nullptr);
        gst_element_link_many(input_gain, in_level, audioconvert, convolver,
                              output_gain, out_level, nullptr);

        auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
        auto pad_src = gst_element_get_static_pad(out_level, "src");

        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
        gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

        gst_object_unref(GST_OBJECT(pad_sink));
        gst_object_unref(GST_OBJECT(pad_src));

        bind_to_gsettings();

        // g_object_set(convolver, "kernelpath", "/home/wallace/dolby.irs",
        //              nullptr);

        // g_signal_connect(settings, "changed::post-messages",
        //                  G_CALLBACK(on_post_messages_changed), this);

        g_settings_bind(settings, "post-messages", in_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);
        g_settings_bind(settings, "post-messages", out_level, "post-messages",
                        G_SETTINGS_BIND_DEFAULT);

        g_settings_bind_with_mapping(
            settings, "input-gain", input_gain, "volume",
            G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear_double,
            util::linear_double_gain_to_db20, nullptr, nullptr);

        g_settings_bind_with_mapping(
            settings, "output-gain", output_gain, "volume",
            G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear_double,
            util::linear_double_gain_to_db20, nullptr, nullptr);

        g_settings_bind(settings, "kernel-path", convolver, "kernel-path",
                        G_SETTINGS_BIND_DEFAULT);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Convolver::~Convolver() {
    util::debug(log_tag + name + " destroyed");
}

void Convolver::bind_to_gsettings() {
    // g_settings_bind_with_mapping(
    //     settings, "input-gain", convolver, "level-in",
    //     G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
    //     util::linear_gain_to_db20, nullptr, nullptr);
    //
    // g_settings_bind_with_mapping(settings, "output-gain", convolver,
    //                              "level-out", G_SETTINGS_BIND_DEFAULT,
    //                              util::db20_gain_to_linear,
    //                              util::linear_gain_to_db20, nullptr,
    //                              nullptr);
}
