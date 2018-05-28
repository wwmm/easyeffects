#include <glibmm/main.h>
#include "util.hpp"
#include "webrtc.hpp"

namespace {

void on_state_changed(GSettings* settings, gchar* key, Webrtc* l) {
    auto enable = g_settings_get_boolean(settings, key);

    if (enable) {
        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "sink"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<Webrtc*>(d);

                auto plugin = gst_bin_get_by_name(
                    GST_BIN(l->plugin), std::string(l->name + "_bin").c_str());

                if (!plugin) {
                    gst_element_unlink(l->identity_in, l->identity_out);

                    gst_bin_add(GST_BIN(l->plugin), l->bin);

                    gst_element_sync_state_with_parent(l->identity_in);
                    gst_element_sync_state_with_parent(l->bin);
                    gst_element_sync_state_with_parent(l->identity_out);

                    gst_element_link_many(l->identity_in, l->bin,
                                          l->identity_out, nullptr);

                    gst_element_set_state(l->probe_bin, GST_STATE_PLAYING);

                    util::debug(l->log_tag + l->name + " enabled");
                }

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    } else {
        gst_pad_add_probe(
            gst_element_get_static_pad(l->identity_in, "sink"),
            GST_PAD_PROBE_TYPE_IDLE,
            [](auto pad, auto info, auto d) {
                auto l = static_cast<Webrtc*>(d);

                auto plugin = gst_bin_get_by_name(
                    GST_BIN(l->plugin), std::string(l->name + "_bin").c_str());

                if (plugin) {
                    gst_element_unlink_many(l->identity_in, l->bin,
                                            l->identity_out, nullptr);

                    gst_bin_remove(GST_BIN(l->plugin), l->bin);

                    gst_element_set_state(l->probe_bin, GST_STATE_NULL);
                    gst_element_set_state(l->bin, GST_STATE_NULL);

                    gst_element_sync_state_with_parent(l->identity_in);
                    gst_element_sync_state_with_parent(l->identity_out);

                    gst_element_link(l->identity_in, l->identity_out);

                    util::debug(l->log_tag + l->name + " disabled");
                }

                return GST_PAD_PROBE_REMOVE;
            },
            l, nullptr);
    }
}

}  // namespace

Webrtc::Webrtc(std::string tag, std::string schema)
    : log_tag(tag), settings(g_settings_new(schema.c_str())) {
    plugin = gst_bin_new(std::string(name + "_plugin").c_str());
    identity_in = gst_element_factory_make("identity", nullptr);
    identity_out = gst_element_factory_make("identity", nullptr);

    gst_bin_add_many(GST_BIN(plugin), identity_in, identity_out, nullptr);
    gst_element_link_many(identity_in, identity_out, nullptr);

    gst_element_add_pad(
        plugin, gst_ghost_pad_new(
                    "sink", gst_element_get_static_pad(identity_in, "sink")));
    gst_element_add_pad(
        plugin, gst_ghost_pad_new(
                    "src", gst_element_get_static_pad(identity_out, "src")));

    webrtc = gst_element_factory_make("webrtcdsp", nullptr);

    if (is_installed(webrtc)) {
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

bool Webrtc::is_installed(GstElement* e) {
    if (e != nullptr) {
        return true;
    } else {
        util::warning(name + " plugin was not found!");

        return false;
    }
}

void Webrtc::build_probe_bin() {
    probe_bin = gst_bin_new("probe_bin");

    probe_src = gst_element_factory_make("pulsesrc", nullptr);
    auto queue = gst_element_factory_make("queue", nullptr);
    auto audioconvert = gst_element_factory_make("audioconvert", nullptr);
    auto audioresample = gst_element_factory_make("audioresample", nullptr);
    auto capsfilter = gst_element_factory_make("capsfilter", nullptr);
    auto probe = gst_element_factory_make("webrtcechoprobe", nullptr);
    auto sink = gst_element_factory_make("fakesink", nullptr);

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
    bin = gst_bin_new("webrtc_bin");

    auto in_level = gst_element_factory_make("level", "webrtc_input_level");
    auto audioconvert_in = gst_element_factory_make("audioconvert", nullptr);
    auto audioresample_in = gst_element_factory_make("audioresample", nullptr);
    auto capsfilter = gst_element_factory_make("capsfilter", nullptr);
    auto audioconvert_out = gst_element_factory_make("audioconvert", nullptr);
    auto audioresample_out = gst_element_factory_make("audioresample", nullptr);
    auto out_level = gst_element_factory_make("level", "webrtc_output_level");

    auto caps_str = "audio/x-raw,format=S16LE,channels=2,rate=48000";

    g_object_set(capsfilter, "caps", gst_caps_from_string(caps_str), nullptr);

    gst_bin_add_many(GST_BIN(bin), probe_bin, in_level, audioconvert_in,
                     audioresample_in, capsfilter, webrtc, audioconvert_out,
                     audioresample_out, out_level, nullptr);

    gst_element_link_many(in_level, audioconvert_in, audioresample_in,
                          capsfilter, webrtc, audioconvert_out,
                          audioresample_out, out_level, nullptr);

    auto pad_sink = gst_element_get_static_pad(in_level, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

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
