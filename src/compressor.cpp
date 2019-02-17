#include "compressor.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Compressor* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->reduction_connection.connected()) {
      l->reduction_connection = Glib::signal_timeout().connect(
          [l]() {
            float compression;

            g_object_get(l->compressor, "rlm", &compression, nullptr);

            l->reduction.emit(compression);

            return true;
          },
          100);
    }
  } else {
    l->reduction_connection.disconnect();
  }
}

}  // namespace

Compressor::Compressor(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "compressor", schema) {
  compressor = gst_element_factory_make(
      "lsp-plug-in-plugins-lv2-compressor-stereo", nullptr);

  if (is_installed(compressor)) {
    auto in_level = gst_element_factory_make("level", "compressor_input_level");
    auto out_level =
        gst_element_factory_make("level", "compressor_output_level");
    auto audioconvert_in =
        gst_element_factory_make("audioconvert", "compressor_audioconvert_in");
    auto audioconvert_out =
        gst_element_factory_make("audioconvert", "compressor_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), in_level, audioconvert_in, compressor,
                     audioconvert_out, out_level, nullptr);

    gst_element_link_many(in_level, audioconvert_in, compressor,
                          audioconvert_out, out_level, nullptr);

    auto pad_sink = gst_element_get_static_pad(in_level, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(compressor, "bypass", false, nullptr);
    g_object_set(compressor, "pause", true, nullptr);  // pause graph analysis
    g_object_set(compressor, "rrl", 0.0f, nullptr);    // relative release level
    g_object_set(compressor, "cdr", 0.0f, nullptr);    // dry gain
    g_object_set(compressor, "cwt", 1.0f, nullptr);    /// wet gain

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages",
                     G_CALLBACK(on_post_messages_changed), this);

    g_settings_bind(settings, "post-messages", in_level, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages",
                    G_SETTINGS_BIND_DEFAULT);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Compressor::~Compressor() {
  util::debug(log_tag + name + " destroyed");
}

void Compressor::bind_to_gsettings() {
  // g_settings_bind(settings, "detection", compressor, "detection",
  //                 G_SETTINGS_BIND_DEFAULT);
  // g_settings_bind(settings, "stereo-link", compressor, "stereo-link",
  //                 G_SETTINGS_BIND_DEFAULT);
  //
  // g_settings_bind_with_mapping(settings, "mix", compressor, "mix",
  //                              G_SETTINGS_BIND_GET,
  //                              util::db20_gain_to_linear, nullptr, nullptr,
  //                              nullptr);

  g_settings_bind_with_mapping(settings, "attack", compressor, "at",
                               G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release", compressor, "rt",
                               G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio", compressor, "cr",
                               G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings, "threshold", compressor, "al", G_SETTINGS_BIND_DEFAULT,
      util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings, "knee", compressor, "kn", G_SETTINGS_BIND_DEFAULT,
      util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings, "makeup", compressor, "mk", G_SETTINGS_BIND_DEFAULT,
      util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings, "preamp", compressor, "scp", G_SETTINGS_BIND_DEFAULT,
      util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "reactivity", compressor, "scr",
                               G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "lookahead", compressor, "sla",
                               G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);
}
