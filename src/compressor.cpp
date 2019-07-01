#include "compressor.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Compressor* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL, inR;

            g_object_get(l->compressor, "ilm-l", &inL, nullptr);
            g_object_get(l->compressor, "ilm-r", &inR, nullptr);

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

            g_object_get(l->compressor, "olm-l", &outL, nullptr);
            g_object_get(l->compressor, "olm-r", &outR, nullptr);

            std::array<double, 2> out_peak = {outL, outR};

            l->output_level.emit(out_peak);

            return true;
          },
          100);
    }

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

    if (!l->sidechain_connection.connected()) {
      l->sidechain_connection = Glib::signal_timeout().connect(
          [l]() {
            float v;

            g_object_get(l->compressor, "slm", &v, nullptr);

            l->sidechain.emit(v);

            return true;
          },
          100);
    }

    if (!l->curve_connection.connected()) {
      l->curve_connection = Glib::signal_timeout().connect(
          [l]() {
            float v;

            g_object_get(l->compressor, "clm", &v, nullptr);

            l->curve.emit(v);

            return true;
          },
          100);
    }
  } else {
    l->input_level_connection.disconnect();
    l->output_level_connection.disconnect();
    l->reduction_connection.disconnect();
    l->sidechain_connection.disconnect();
    l->curve_connection.disconnect();
  }
}

}  // namespace

Compressor::Compressor(const std::string& tag, const std::string& schema) : PluginBase(tag, "compressor", schema) {
  compressor = gst_element_factory_make("lsp-plug-in-plugins-lv2-compressor-stereo", nullptr);

  if (is_installed(compressor)) {
    auto audioconvert_in = gst_element_factory_make("audioconvert", "compressor_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "compressor_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, compressor, audioconvert_out, nullptr);

    gst_element_link_many(audioconvert_in, compressor, audioconvert_out, nullptr);

    auto pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto pad_src = gst_element_get_static_pad(audioconvert_out, "src");

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

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Compressor::~Compressor() {
  util::debug(log_tag + name + " destroyed");
}

void Compressor::bind_to_gsettings() {
  g_settings_bind(settings, "mode", compressor, "cm", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "sidechain-listen", compressor, "scl", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "sidechain-type", compressor, "sct", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "sidechain-mode", compressor, "scm", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "sidechain-source", compressor, "scs", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "input-gain", compressor, "g-in", G_SETTINGS_BIND_GET,
                               util::db20_gain_to_linear, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", compressor, "g-out", G_SETTINGS_BIND_GET,
                               util::db20_gain_to_linear, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "attack", compressor, "at", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release", compressor, "rt", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio", compressor, "cr", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "threshold", compressor, "al", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "knee", compressor, "kn", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "makeup", compressor, "mk", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "sidechain-preamp", compressor, "scp", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "sidechain-reactivity", compressor, "scr", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "sidechain-lookahead", compressor, "sla", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);
}
