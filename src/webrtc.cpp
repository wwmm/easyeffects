#include <glibmm/main.h>
#include <gst/insertbin/gstinsertbin.h>
#include "util.hpp"
#include "webrtc.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Webrtc* l) {
    auto enable = g_settings_get_boolean(settings, key);
    auto plugin = gst_bin_get_by_name(GST_BIN(l->plugin), "webrtc_bin");

    if (enable) {
        if (!plugin) {
            gst_insert_bin_append(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Webrtc*>(d);

                    if (success) {
                        util::debug(l->log_tag + "webrtc enabled");
                    } else {
                        util::debug(l->log_tag + "failed to enable the webrtc");
                    }
                },
                l);
        }
    } else {
        if (plugin) {
            gst_insert_bin_remove(
                GST_INSERT_BIN(l->plugin), l->bin,
                [](auto bin, auto elem, auto success, auto d) {
                    auto l = static_cast<Webrtc*>(d);

                    if (success) {
                        util::debug(l->log_tag + "webrtc disabled");
                    } else {
                        util::debug(l->log_tag +
                                    "failed to disable the webrtc");
                    }
                },
                l);
        }
    }
}

}  // namespace

Webrtc::Webrtc(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    webrtc = gst_element_factory_make("webrtcdsp", nullptr);

    plugin = gst_insert_bin_new("webrtc_plugin");

    if (webrtc != nullptr) {
        is_installed = true;
    } else {
        is_installed = false;

        util::warning("Webrtc plugin was not found!");
    }

    if (is_installed) {
        build_probe_bin();
        build_dsp_bin();

        bind_to_gsettings();

        g_signal_connect(settings, "changed::state",
                         G_CALLBACK(on_state_changed), this);

        // useless write just to force callback call

        auto enable = g_settings_get_boolean(settings, "state");

        g_settings_set_boolean(settings, "state", enable);
    }
}

Webrtc::~Webrtc() {}

void Webrtc::build_probe_bin() {
    probe_bin = gst_bin_new("probe_bin");

    probe_src = gst_element_factory_make("pulsesrc", nullptr);
    auto queue = gst_element_factory_make("queue", nullptr);
    auto audioconvert = gst_element_factory_make("audioconvert", nullptr);
    auto audioresample = gst_element_factory_make("audioresample", nullptr);
    auto capsfilter = gst_element_factory_make("capsfilter", nullptr);
    auto probe = gst_element_factory_make("webrtcechoprobe", nullptr);
    auto sink = gst_element_factory_make("pulsesink", nullptr);

    auto props = gst_structure_from_string(
        "props,application.name=PulseEffectsWebrtcProbe", nullptr);

    auto caps_str = "audio/x-raw,format=S16LE,channels=2,rate=48000";

    g_object_set(probe_src, "stream-properties", props, nullptr);
    g_object_set(probe_src, "buffer-time", 10000, nullptr);
    g_object_set(capsfilter, "caps", gst_caps_from_string(caps_str), nullptr);
    g_object_set(queue, "silent", true, nullptr);

    gst_bin_add_many(GST_BIN(probe_bin), probe_src, queue, audioconvert,
                     audioresample, capsfilter, probe, sink, nullptr);

    gst_element_link_many(probe_src, queue, audioconvert, audioresample,
                          capsfilter, probe, sink, nullptr);
}

void Webrtc::build_dsp_bin() {
    bin = gst_insert_bin_new("webrtc_bin");

    auto in_level = gst_element_factory_make("level", "webrtc_input_level");
    auto audioconvert_in = gst_element_factory_make("audioconvert", nullptr);
    auto audioresample_in = gst_element_factory_make("audioresample", nullptr);
    auto capsfilter = gst_element_factory_make("capsfilter", nullptr);
    auto audioconvert_out = gst_element_factory_make("audioconvert", nullptr);
    auto audioresample_out = gst_element_factory_make("audioresample", nullptr);
    auto out_level = gst_element_factory_make("level", "webrtc_output_level");

    auto caps_str = "audio/x-raw,format=S16LE,channels=2,rate=48000";

    g_object_set(capsfilter, "caps", gst_caps_from_string(caps_str), nullptr);

    gst_insert_bin_append(GST_INSERT_BIN(bin), in_level, nullptr, nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert_in, nullptr,
                          nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), audioresample_in, nullptr,
                          nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), capsfilter, nullptr, nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), webrtc, nullptr, nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), audioconvert_out, nullptr,
                          nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), audioresample_out, nullptr,
                          nullptr);
    gst_insert_bin_append(GST_INSERT_BIN(bin), out_level, nullptr, nullptr);

    g_settings_bind(settings, "post-messages", in_level, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);
}

void Webrtc::set_probe_src_device(std::string name) {
    if (probe_src) {
        g_object_set(probe_src, "device", name.c_str(), nullptr);
    }
}

void Webrtc::bind_to_gsettings() {
    g_settings_bind(settings, "high-pass-filter", webrtc, "high-pass-filter",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "echo-cancel", webrtc, "echo-cancel",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "echo-suppression-level", webrtc,
                    "echo-suppression-level", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "noise-suppression", webrtc, "noise-suppression",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "noise-suppression-level", webrtc,
                    "noise-suppression-level", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "gain-control", webrtc, "gain-control",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "extended-filter", webrtc, "extended-filter",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "delay-agnostic", webrtc, "delay-agnostic",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "target-level-dbfs", webrtc, "target-level-dbfs",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "compression-gain-db", webrtc,
                    "compression-gain-db", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "limiter", webrtc, "limiter",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "voice-detection", webrtc, "voice-detection",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "gain-control-mode", webrtc, "gain-control-mode",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "voice-detection-frame-size-ms", webrtc,
                    "voice-detection-frame-size-ms", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(settings, "voice-detection-likelihood", webrtc,
                    "voice-detection-likelihood", G_SETTINGS_BIND_DEFAULT);
}
