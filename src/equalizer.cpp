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
  equalizer = gst_element_factory_make(
      "lsp-plug-in-plugins-lv2-para-equalizer-x32-lr", nullptr);

  if (is_installed(equalizer)) {
    auto input_gain = gst_element_factory_make("volume", nullptr);
    auto in_level = gst_element_factory_make("level", "equalizer_input_level");
    auto out_level =
        gst_element_factory_make("level", "equalizer_output_level");
    auto output_gain = gst_element_factory_make("volume", nullptr);

    auto audioconvert_in =
        gst_element_factory_make("audioconvert", "eq_audioconvert_in");
    auto audioconvert_out =
        gst_element_factory_make("audioconvert", "eq_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in,
                     equalizer, audioconvert_out, output_gain, out_level,
                     nullptr);

    gst_element_link_many(input_gain, in_level, audioconvert_in, equalizer,
                          audioconvert_out, output_gain, out_level, nullptr);

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

    for (int n = 0; n < 30; n++) {
      bind_band(equalizer, n);
    }

    // connect signals

    g_signal_connect(settings, "changed::num-bands",
                     G_CALLBACK(on_num_bands_changed), this);

    // gsettings bindings

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

  g_settings_bind(
      settings_left,
      std::string("band" + std::to_string(index) + "-type").c_str(), equalizer,
      std::string("ftl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET);

  g_settings_bind_with_mapping(
      settings_left,
      std::string("band" + std::to_string(index) + "-frequency").c_str(),
      equalizer, std::string("fl-" + std::to_string(index)).c_str(),
      G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings_left, std::string("band" + std::to_string(index) + "-q").c_str(),
      equalizer, std::string("ql-" + std::to_string(index)).c_str(),
      G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings_left,
      std::string("band" + std::to_string(index) + "-gain").c_str(), equalizer,
      std::string("gl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
      util::db20_gain_to_linear, nullptr, nullptr, nullptr);

  // right channel

  g_settings_bind(
      settings_right,
      std::string("band" + std::to_string(index) + "-type").c_str(), equalizer,
      std::string("ftr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET);

  g_settings_bind_with_mapping(
      settings_right,
      std::string("band" + std::to_string(index) + "-frequency").c_str(),
      equalizer, std::string("fr-" + std::to_string(index)).c_str(),
      G_SETTINGS_BIND_GET, util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings_right,
      std::string("band" + std::to_string(index) + "-q").c_str(), equalizer,
      std::string("qr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
      util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(
      settings_right,
      std::string("band" + std::to_string(index) + "-gain").c_str(), equalizer,
      std::string("gr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
      util::db20_gain_to_linear, nullptr, nullptr, nullptr);
}

void Equalizer::unbind_band(GstElement* equalizer, const int index) {
  // auto band =
  //     gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer), index);
  //
  // g_settings_unbind(
  //     band, std::string("band" + std::to_string(index) + "-gain").c_str());
  //
  // g_settings_unbind(
  //     band, std::string("band" + std::to_string(index) +
  //     "-frequency").c_str());
  //
  // g_settings_unbind(
  //     band, std::string("band" + std::to_string(index) + "-width").c_str());
  //
  // g_settings_unbind(
  //     band, std::string("band" + std::to_string(index) + "-type").c_str());
  //
  // g_object_unref(band);
}

void Equalizer::update_equalizer() {
  // int nbands = g_settings_get_int(settings, "num-bands");
  // int current_nbands;

  // g_object_get(equalizer_L, "num-bands", &current_nbands, nullptr);

  // if (nbands != current_nbands) {
  //   util::debug(log_tag + name + ": unbinding bands");
  //
  //   for (int n = 0; n < current_nbands; n++) {
  //     unbind_band(equalizer_L, n);
  //     unbind_band(equalizer_R, n);
  //   }
  //
  //   util::debug(log_tag + name + ": setting new number of bands");
  //
  //   g_object_set(equalizer_L, "num-bands", nbands, nullptr);
  //   g_object_set(equalizer_R, "num-bands", nbands, nullptr);
  //
  //   util::debug(log_tag + name + ": binding bands");
  //
  //   for (int n = 0; n < nbands; n++) {
  //     bind_band(equalizer_L, settings_left, n);
  //     bind_band(equalizer_R, settings_right, n);
  //   }
  // }
}
