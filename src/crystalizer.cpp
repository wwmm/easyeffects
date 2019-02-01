#include "crystalizer.hpp"
#include <glibmm/main.h>
#include "util.hpp"

Crystalizer::Crystalizer(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "crystalizer", schema) {
  crystalizer_low = gst_element_factory_make("pecrystalizer", nullptr);
  crystalizer_mid = gst_element_factory_make("pecrystalizer", nullptr);
  crystalizer_high = gst_element_factory_make("pecrystalizer", nullptr);

  if (is_installed(crystalizer_low)) {
    auto input_gain = gst_element_factory_make("volume", nullptr);
    auto in_level =
        gst_element_factory_make("level", "crystalizer_input_level");
    auto output_gain = gst_element_factory_make("volume", nullptr);
    auto out_level =
        gst_element_factory_make("level", "crystalizer_output_level");
    auto audioconvert_in =
        gst_element_factory_make("audioconvert", "crystalizer_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make(
        "audioconvert", "crystalizer_audioconvert_out");

    auto queue_low = gst_element_factory_make("queue", "crystalizer_queue0");
    auto queue_mid = gst_element_factory_make("queue", "crystalizer_queue1");
    auto queue_high = gst_element_factory_make("queue", "crystalizer_queue2");

    tee = gst_element_factory_make("tee", "crystalizer_tee");
    mixer = gst_element_factory_make("audiomixer", "crystalizer_mixer");
    lowpass = gst_element_factory_make("audiocheblimit", "crystalizer_lowpass");
    bandpass =
        gst_element_factory_make("audiochebband", "crystalizer_bandpass");
    highpass =
        gst_element_factory_make("audiocheblimit", "crystalizer_highpass");

    g_object_set(queue_low, "silent", true, nullptr);
    g_object_set(queue_low, "flush-on-eos", true, nullptr);
    g_object_set(queue_low, "max-size-buffers", 0, nullptr);
    g_object_set(queue_low, "max-size-bytes", 0, nullptr);
    g_object_set(queue_low, "max-size-time", 0, nullptr);

    g_object_set(queue_high, "silent", true, nullptr);
    g_object_set(queue_high, "flush-on-eos", true, nullptr);
    g_object_set(queue_high, "max-size-buffers", 0, nullptr);
    g_object_set(queue_high, "max-size-bytes", 0, nullptr);
    g_object_set(queue_high, "max-size-time", 0, nullptr);

    g_object_set(lowpass, "mode", 0, nullptr);
    g_object_set(lowpass, "type", 1, nullptr);
    g_object_set(lowpass, "poles", 16, nullptr);
    g_object_set(lowpass, "ripple", 0, nullptr);

    g_object_set(bandpass, "mode", 0, nullptr);
    g_object_set(bandpass, "type", 1, nullptr);
    g_object_set(bandpass, "poles", 16, nullptr);
    g_object_set(bandpass, "ripple", 0, nullptr);

    g_object_set(highpass, "mode", 1, nullptr);
    g_object_set(highpass, "type", 1, nullptr);
    g_object_set(highpass, "poles", 16, nullptr);
    g_object_set(highpass, "ripple", 0, nullptr);

    g_object_set(lowpass, "cutoff", 3000.0f, nullptr);
    g_object_set(bandpass, "lower-frequency", 3000.0f, nullptr);
    g_object_set(bandpass, "upper-frequency", 10000.0f, nullptr);
    g_object_set(highpass, "cutoff", 10000.0f, nullptr);

    g_object_set(mixer, "start-time-selection", 1, nullptr);

    tee_src0 = gst_element_get_request_pad(tee, "src_0");
    tee_src1 = gst_element_get_request_pad(tee, "src_1");
    tee_src2 = gst_element_get_request_pad(tee, "src_2");
    mixer_sink0 = gst_element_get_request_pad(mixer, "sink_0");
    mixer_sink1 = gst_element_get_request_pad(mixer, "sink_1");
    mixer_sink2 = gst_element_get_request_pad(mixer, "sink_2");

    auto queue_low_sink = gst_element_get_static_pad(queue_low, "sink");
    auto queue_mid_sink = gst_element_get_static_pad(queue_mid, "sink");
    auto queue_high_sink = gst_element_get_static_pad(queue_high, "sink");

    auto crystalizer_low_src =
        gst_element_get_static_pad(crystalizer_low, "src");
    auto crystalizer_mid_src =
        gst_element_get_static_pad(crystalizer_mid, "src");
    auto crystalizer_high_src =
        gst_element_get_static_pad(crystalizer_high, "src");

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, tee,
                     queue_low, queue_mid, queue_high, lowpass, bandpass,
                     highpass, crystalizer_low, crystalizer_mid,
                     crystalizer_high, mixer, audioconvert_out, output_gain,
                     out_level, nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, tee, nullptr);

    gst_pad_link(tee_src0, queue_low_sink);
    gst_pad_link(tee_src1, queue_mid_sink);
    gst_pad_link(tee_src2, queue_high_sink);

    gst_element_link_many(queue_low, lowpass, crystalizer_low, nullptr);
    gst_element_link_many(queue_mid, bandpass, crystalizer_mid, nullptr);
    gst_element_link_many(queue_high, highpass, crystalizer_high, nullptr);

    gst_pad_link(crystalizer_low_src, mixer_sink0);
    gst_pad_link(crystalizer_mid_src, mixer_sink1);
    gst_pad_link(crystalizer_high_src, mixer_sink2);

    gst_element_link_many(mixer, audioconvert_out, output_gain, out_level,
                          nullptr);

    auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));
    gst_object_unref(GST_OBJECT(queue_low_sink));
    gst_object_unref(GST_OBJECT(queue_mid_sink));
    gst_object_unref(GST_OBJECT(queue_high_sink));
    gst_object_unref(GST_OBJECT(crystalizer_low_src));
    gst_object_unref(GST_OBJECT(crystalizer_mid_src));
    gst_object_unref(GST_OBJECT(crystalizer_high_src));

    bind_to_gsettings();

    g_settings_bind(settings, "post-messages", in_level, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(
        settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear_double, util::linear_double_gain_to_db20,
        nullptr, nullptr);

    g_settings_bind_with_mapping(
        settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
        util::db20_gain_to_linear_double, util::linear_double_gain_to_db20,
        nullptr, nullptr);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Crystalizer::~Crystalizer() {
  util::debug(log_tag + name + " destroyed");

  gst_element_release_request_pad(tee, tee_src0);
  gst_element_release_request_pad(tee, tee_src1);
  gst_element_release_request_pad(mixer, mixer_sink0);
  gst_element_release_request_pad(mixer, mixer_sink1);

  gst_object_unref(GST_OBJECT(tee_src0));
  gst_object_unref(GST_OBJECT(tee_src1));
  gst_object_unref(GST_OBJECT(mixer_sink0));
  gst_object_unref(GST_OBJECT(mixer_sink1));
}

void Crystalizer::bind_to_gsettings() {
  g_settings_bind_with_mapping(
      settings, "intensity-low", crystalizer_low, "intensity",
      G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings, "intensity-mid", crystalizer_mid, "intensity",
      G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings, "intensity-high", crystalizer_high, "intensity",
      G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);
}
