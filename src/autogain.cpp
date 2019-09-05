#include "autogain.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_m_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "m", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->momentary.emit(v); });
}

void on_s_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "s", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->shortterm.emit(v); });
}

void on_i_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "i", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->integrated.emit(v); });
}

void on_r_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "r", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->relative.emit(v); });
}

void on_l_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "l", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->loudness.emit(v); });
}

void on_lra_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "lra", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->range.emit(v); });
}

void on_g_changed(GObject* gobject, GParamSpec* pspec, AutoGain* a) {
  float v;

  g_object_get(a->autogain, "g", &v, nullptr);

  Glib::signal_idle().connect_once([=] { a->gain.emit(v); });
}

}  // namespace

AutoGain::AutoGain(const std::string& tag, const std::string& schema) : PluginBase(tag, "autogain", schema) {
  autogain = gst_element_factory_make("peautogain", nullptr);

  if (is_installed(autogain)) {
    auto input_gain = gst_element_factory_make("volume", nullptr);
    auto in_level = gst_element_factory_make("level", "autogain_input_level");
    auto output_gain = gst_element_factory_make("volume", nullptr);
    auto out_level = gst_element_factory_make("level", "autogain_output_level");
    auto audioconvert_in = gst_element_factory_make("audioconvert", "autogain_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "autogain_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, autogain, audioconvert_out, output_gain,
                     out_level, nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, autogain, audioconvert_out, output_gain, out_level,
                          nullptr);

    auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    bind_to_gsettings();

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", autogain, "notify-host", G_SETTINGS_BIND_DEFAULT);

    g_signal_connect(autogain, "notify::m", G_CALLBACK(on_m_changed), this);
    g_signal_connect(autogain, "notify::s", G_CALLBACK(on_s_changed), this);
    g_signal_connect(autogain, "notify::i", G_CALLBACK(on_i_changed), this);
    g_signal_connect(autogain, "notify::r", G_CALLBACK(on_r_changed), this);
    g_signal_connect(autogain, "notify::l", G_CALLBACK(on_l_changed), this);
    g_signal_connect(autogain, "notify::lra", G_CALLBACK(on_lra_changed), this);
    g_signal_connect(autogain, "notify::g", G_CALLBACK(on_g_changed), this);

    g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

AutoGain::~AutoGain() {
  util::debug(log_tag + name + " destroyed");
}

void AutoGain::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "target", autogain, "target", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind(settings, "weight-m", autogain, "weight-m", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "weight-s", autogain, "weight-s", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "weight-i", autogain, "weight-i", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "detect-silence", autogain, "detect-silence", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "reset", autogain, "reset", G_SETTINGS_BIND_DEFAULT);
}
