#include "stereo_tools.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, StereoTools* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL, inR;

            g_object_get(l->stereo_tools, "meter-inL", &inL, nullptr);
            g_object_get(l->stereo_tools, "meter-inR", &inR, nullptr);

            std::array<double, 2> in_peak = {inL, inR};

            l->input_level.emit(in_peak);

            return true;
          },
          100);
    }

    if (!l->output_level_connection.connected()) {
      l->output_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float outL, outR;

            g_object_get(l->stereo_tools, "meter-outL", &outL, nullptr);
            g_object_get(l->stereo_tools, "meter-outR", &outR, nullptr);

            std::array<double, 2> out_peak = {outL, outR};

            l->output_level.emit(out_peak);

            return true;
          },
          100);
    }
  } else {
    l->input_level_connection.disconnect();
    l->output_level_connection.disconnect();
  }
}

}  // namespace

StereoTools::StereoTools(const std::string& tag, const std::string& schema) : PluginBase(tag, "stereo_tools", schema) {
  stereo_tools = gst_element_factory_make("calf-sourceforge-net-plugins-StereoTools", "stereo_tools");

  if (is_installed(stereo_tools)) {
    auto audioconvert_in = gst_element_factory_make("audioconvert", "stereo_tools_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "stereo_tools_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, stereo_tools, audioconvert_out, nullptr);

    gst_element_link_many(audioconvert_in, stereo_tools, audioconvert_out, nullptr);

    auto pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto pad_src = gst_element_get_static_pad(audioconvert_out, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(stereo_tools, "bypass", false, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

StereoTools::~StereoTools() {
  util::debug(log_tag + name + " destroyed");
}

void StereoTools::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", stereo_tools, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", stereo_tools, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "balance-in", stereo_tools, "balance-in", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "balance-out", stereo_tools, "balance-out", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind(settings, "softclip", stereo_tools, "softclip", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "mutel", stereo_tools, "mutel", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "muter", stereo_tools, "muter", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "phasel", stereo_tools, "phasel", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "phaser", stereo_tools, "phaser", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "mode", stereo_tools, "mode", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "slev", stereo_tools, "slev", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "sbal", stereo_tools, "sbal", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "mlev", stereo_tools, "mlev", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "mpan", stereo_tools, "mpan", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "stereo-base", stereo_tools, "stereo-base", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "delay", stereo_tools, "delay", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "sc-level", stereo_tools, "sc-level", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "stereo-phase", stereo_tools, "stereo-phase", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);
}
