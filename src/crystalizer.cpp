#include "crystalizer.hpp"
#include <glibmm/main.h>
#include "util.hpp"

Crystalizer::Crystalizer(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "crystalizer", schema) {
  crystalizer_low = gst_element_factory_make("pecrystalizer", nullptr);
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

    auto tee = gst_element_factory_make("tee", "crystalizer_tee");
    auto queue_low = gst_element_factory_make("queue", "crystalizer_queue0");
    auto queue_high = gst_element_factory_make("queue", "crystalizer_queue1");
    auto lowpass =
        gst_element_factory_make("audiocheblimit", "crystalizer_lowpass");
    auto highpass =
        gst_element_factory_make("audiocheblimit", "crystalizer_highpass");
    auto mixer = gst_element_factory_make("audiomixer", "crystalizer_mixer");

    g_object_set(queue_low, "silent", true, nullptr);
    g_object_set(queue_low, "max-size-buffers", 1, nullptr);
    g_object_set(queue_low, "max-size-bytes", 0, nullptr);
    g_object_set(queue_low, "max-size-time", 0, nullptr);

    g_object_set(queue_high, "silent", true, nullptr);
    g_object_set(queue_high, "max-size-buffers", 1, nullptr);
    g_object_set(queue_high, "max-size-bytes", 0, nullptr);
    g_object_set(queue_high, "max-size-time", 0, nullptr);

    g_object_set(lowpass, "mode", 0, nullptr);
    g_object_set(lowpass, "type", 1, nullptr);
    g_object_set(lowpass, "poles", 16, nullptr);
    g_object_set(lowpass, "ripple", 0, nullptr);

    g_object_set(highpass, "mode", 1, nullptr);
    g_object_set(highpass, "type", 1, nullptr);
    g_object_set(highpass, "poles", 16, nullptr);
    g_object_set(highpass, "ripple", 0, nullptr);

    g_object_set(lowpass, "cutoff", 12000.0f, nullptr);
    g_object_set(highpass, "cutoff", 12000.0f, nullptr);
    g_object_set(crystalizer_high, "intensity", 0.5f, nullptr);

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, tee,
                     queue_low, queue_high, lowpass, highpass, crystalizer_low,
                     crystalizer_high, mixer, audioconvert_out, output_gain,
                     out_level, nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, tee, nullptr);

    gst_element_link_many(tee, queue_low, lowpass, crystalizer_low, mixer,
                          nullptr);
    gst_element_link_many(tee, queue_high, highpass, crystalizer_high, mixer,
                          nullptr);

    gst_element_link_many(mixer, audioconvert_out, output_gain, out_level,
                          nullptr);

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
}

void Crystalizer::bind_to_gsettings() {
  g_settings_bind_with_mapping(
      settings, "intensity", crystalizer_low, "intensity", G_SETTINGS_BIND_GET,
      util::double_to_float, nullptr, nullptr, nullptr);
}
