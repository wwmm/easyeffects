#include "crystalizer.hpp"
#include <glibmm/main.h>
#include "util.hpp"

Crystalizer::Crystalizer(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "crystalizer", schema) {
  crystalizer = gst_element_factory_make("pecrystalizer", nullptr);

  if (is_installed(crystalizer)) {
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

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in,
                     crystalizer, audioconvert_out, output_gain, out_level,
                     nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, crystalizer,
                          audioconvert_out, output_gain, out_level, nullptr);

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
  for (int n = 0; n < 12; n++) {
    g_settings_bind_with_mapping(
        settings, std::string("intensity-band" + std::to_string(n)).c_str(),
        crystalizer, std::string("intensity-band" + std::to_string(n)).c_str(),
        G_SETTINGS_BIND_GET, util::db20_gain_to_linear, nullptr, nullptr,
        nullptr);

    g_settings_bind(
        settings, std::string("mute-band" + std::to_string(n)).c_str(),
        crystalizer, std::string("mute-band" + std::to_string(n)).c_str(),
        G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(
        settings, std::string("bypass-band" + std::to_string(n)).c_str(),
        crystalizer, std::string("bypass-band" + std::to_string(n)).c_str(),
        G_SETTINGS_BIND_DEFAULT);
  }
}
