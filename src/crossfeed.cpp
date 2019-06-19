#include "crossfeed.hpp"
#include <glibmm/main.h>
#include "util.hpp"

Crossfeed::Crossfeed(const std::string& tag, const std::string& schema) : PluginBase(tag, "crossfeed", schema) {
  crossfeed = gst_element_factory_make("bs2b", nullptr);

  if (is_installed(crossfeed)) {
    auto in_level = gst_element_factory_make("level", "crossfeed_input_level");
    auto out_level = gst_element_factory_make("level", "crossfeed_output_level");
    auto audioconvert_in = gst_element_factory_make("audioconvert", "crossfeed_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "crossfeed_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), in_level, audioconvert_in, crossfeed, audioconvert_out, out_level, nullptr);

    gst_element_link_many(in_level, audioconvert_in, crossfeed, audioconvert_out, out_level, nullptr);

    auto pad_sink = gst_element_get_static_pad(in_level, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    bind_to_gsettings();

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Crossfeed::~Crossfeed() {
  util::debug(log_tag + name + " destroyed");
}

void Crossfeed::bind_to_gsettings() {
  g_settings_bind(settings, "fcut", crossfeed, "fcut", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "feed", crossfeed, "feed", G_SETTINGS_BIND_GET, util::double_x10_to_int,
                               nullptr, nullptr, nullptr);
}
