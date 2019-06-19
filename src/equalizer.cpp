#include "equalizer.hpp"
#include <glibmm/main.h>
#include <chrono>
#include "util.hpp"

namespace {

void on_num_bands_changed(GSettings* settings, gchar* key, Equalizer* l) {
  l->update_equalizer();
}

}  // namespace

Equalizer::Equalizer(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_left,
                     const std::string& schema_right)
    : PluginBase(tag, "equalizer", schema),
      settings_left(g_settings_new(schema_left.c_str())),
      settings_right(g_settings_new(schema_right.c_str())) {
  equalizer = gst_element_factory_make("lsp-plug-in-plugins-lv2-para-equalizer-x32-lr", nullptr);

  if (is_installed(equalizer)) {
    auto input_gain = gst_element_factory_make("volume", nullptr);
    auto in_level = gst_element_factory_make("level", "equalizer_input_level");
    auto out_level = gst_element_factory_make("level", "equalizer_output_level");
    auto output_gain = gst_element_factory_make("volume", nullptr);

    auto audioconvert_in = gst_element_factory_make("audioconvert", "eq_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "eq_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, equalizer, audioconvert_out, output_gain,
                     out_level, nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, equalizer, audioconvert_out, output_gain, out_level,
                          nullptr);

    // setting bin ghost pads

    auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    // init

    g_object_set(equalizer, "bypass", false, nullptr);
    g_object_set(equalizer, "bal", 0.0f, nullptr);
    g_object_set(equalizer, "fft", 0, nullptr);  // off

    for (int n = 0; n < 30; n++) {
      bind_band(equalizer, n);
    }

    // connect signals

    g_signal_connect(settings, "changed::num-bands", G_CALLBACK(on_num_bands_changed), this);

    // gsettings bindings

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
                                 util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr, nullptr);

    g_settings_bind(settings, "mode", equalizer, "mode", G_SETTINGS_BIND_DEFAULT);

    // useless write just to force on_state_changed callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Equalizer::~Equalizer() {
  g_object_unref(settings_left);
  g_object_unref(settings_right);

  util::debug(log_tag + name + " destroyed");
}

void Equalizer::bind_band(GstElement* equalizer, const int index) {
  // left channel

  g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-type").c_str(), equalizer,
                  std::string("ftl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-mode").c_str(), equalizer,
                  std::string("fml-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-slope").c_str(), equalizer,
                  std::string("sl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-solo").c_str(), equalizer,
                  std::string("xsl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-mute").c_str(), equalizer,
                  std::string("xml-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings_left, std::string("band" + std::to_string(index) + "-frequency").c_str(),
                               equalizer, std::string("fl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings_left, std::string("band" + std::to_string(index) + "-q").c_str(), equalizer,
                               std::string("ql-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings_left, std::string("band" + std::to_string(index) + "-gain").c_str(), equalizer,
                               std::string("gl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
                               util::db20_gain_to_linear, nullptr, nullptr, nullptr);

  // right channel

  g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-type").c_str(), equalizer,
                  std::string("ftr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-mode").c_str(), equalizer,
                  std::string("fmr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-slope").c_str(), equalizer,
                  std::string("sr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-solo").c_str(), equalizer,
                  std::string("xsr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-mute").c_str(), equalizer,
                  std::string("xmr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-frequency").c_str(),
                               equalizer, std::string("fr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-q").c_str(), equalizer,
                               std::string("qr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-gain").c_str(), equalizer,
                               std::string("gr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
                               util::db20_gain_to_linear, nullptr, nullptr, nullptr);
}

void Equalizer::update_equalizer() {
  int nbands = g_settings_get_int(settings, "num-bands");

  for (int n = nbands; n < 30; n++) {
    // turn off unused band

    g_object_set(equalizer, std::string("ftl-" + std::to_string(n)).c_str(), 0, nullptr);

    g_object_set(equalizer, std::string("ftr-" + std::to_string(n)).c_str(), 0, nullptr);
  }
}
