#include "equalizer.hpp"
#include <glibmm/main.h>
#include <chrono>
#include "util.hpp"

namespace {

void on_num_bands_changed(GSettings* settings, gchar* key, Equalizer* l) {
  l->update_equalizer();
}

void on_deinterleave_pad_added(GstElement*, GstPad* pad, Equalizer* l) {
  auto name = GST_PAD_NAME(pad);

  util::debug(l->log_tag + l->name + " deinterleave pad added: " + name);

  if (name == std::string("src_0")) {
    auto sinkpad = gst_element_get_static_pad(l->queue_L, "sink");

    gst_pad_link(pad, sinkpad);

    gst_object_unref(GST_OBJECT(sinkpad));
  } else if (name == std::string("src_1")) {
    auto sinkpad = gst_element_get_static_pad(l->queue_R, "sink");

    gst_pad_link(pad, sinkpad);

    gst_object_unref(GST_OBJECT(sinkpad));
  }
}

void on_deinterleave_pad_removed(GstElement*, GstPad* pad, Equalizer* l) {
  std::string name = GST_PAD_NAME(pad);

  util::debug(l->log_tag + l->name + " deinterleave pad removed: " + name);

  if (name == std::string("src_0")) {
    auto sinkpad = gst_element_get_static_pad(l->queue_L, "sink");

    gst_pad_unlink(pad, sinkpad);

    gst_object_unref(GST_OBJECT(sinkpad));
  } else if (name == std::string("src_1")) {
    auto sinkpad = gst_element_get_static_pad(l->queue_R, "sink");

    gst_pad_unlink(pad, sinkpad);

    gst_object_unref(GST_OBJECT(sinkpad));
  }
}

void on_deinterleave_no_more_pads(GstElement*, Equalizer* l) {
  auto b = gst_bin_get_by_name(GST_BIN(l->bin), "eq_interleave");

  if (b) {
    gst_bin_remove(GST_BIN(l->bin), b);
  }

  l->interleave = gst_element_factory_make("interleave", "eq_interleave");

  gst_bin_add(GST_BIN(l->bin), l->interleave);

  gst_element_link(l->interleave, l->audioconvert_out);

  auto interleave_sink0_pad =
      gst_element_get_request_pad(l->interleave, "sink_0");

  auto interleave_sink1_pad =
      gst_element_get_request_pad(l->interleave, "sink_1");

  auto eq_L_src_pad = gst_element_get_static_pad(l->equalizer_L, "src");

  auto eq_R_src_pad = gst_element_get_static_pad(l->equalizer_R, "src");

  gst_pad_link(eq_L_src_pad, interleave_sink0_pad);
  gst_pad_link(eq_R_src_pad, interleave_sink1_pad);

  gst_element_sync_state_with_parent(l->interleave);

  gst_object_unref(GST_OBJECT(interleave_sink0_pad));
  gst_object_unref(GST_OBJECT(interleave_sink1_pad));
  gst_object_unref(GST_OBJECT(eq_L_src_pad));
  gst_object_unref(GST_OBJECT(eq_R_src_pad));
}

}  // namespace

Equalizer::Equalizer(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_left,
                     const std::string& schema_right)
    : PluginBase(tag, "equalizer", schema),
      settings_left(g_settings_new(schema_left.c_str())),
      settings_right(g_settings_new(schema_right.c_str())) {
  equalizer_L = gst_element_factory_make("equalizer-nbands", nullptr);
  equalizer_R = gst_element_factory_make("equalizer-nbands", nullptr);

  if (is_installed(equalizer_L)) {
    auto input_gain = gst_element_factory_make("volume", nullptr);
    auto in_level = gst_element_factory_make("level", "equalizer_input_level");
    auto out_level =
        gst_element_factory_make("level", "equalizer_output_level");
    auto output_gain = gst_element_factory_make("volume", nullptr);
    auto audioconvert_in =
        gst_element_factory_make("audioconvert", "eq_audioconvert_in");
    auto deinterleave =
        gst_element_factory_make("deinterleave", "eq_deinterleave");

    queue_L = gst_element_factory_make("queue", "eq_queue_L");
    queue_R = gst_element_factory_make("queue", "eq_queue_R");
    audioconvert_out =
        gst_element_factory_make("audioconvert", "eq_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in,
                     deinterleave, queue_L, queue_R, equalizer_L, equalizer_R,
                     audioconvert_out, output_gain, out_level, nullptr);

    /*
      Once the pipeline is running we will have the following link order:
                            input_gain
                            in_level
                            audioconvert_in
                            deinterleave
                            /           \
                         queue_L      queue_R
                       equalizer_L  equalizer_R
                              \        /
                              interleave
                              audioconvert_out
                              output_gain
                              out_level
    */

    gst_element_link_many(input_gain, in_level, audioconvert_in, deinterleave,
                          nullptr);

    gst_element_link_many(audioconvert_out, output_gain, out_level, nullptr);

    gst_element_link_many(queue_L, equalizer_L, nullptr);
    gst_element_link_many(queue_R, equalizer_R, nullptr);

    // setting bin ghost pads

    auto pad_sink = gst_element_get_static_pad(input_gain, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    // init

    g_object_set(deinterleave, "keep-positions", true, nullptr);

    int nbands = g_settings_get_int(settings, "num-bands");

    g_object_set(equalizer_L, "num-bands", nbands, nullptr);
    g_object_set(equalizer_R, "num-bands", nbands, nullptr);

    for (int n = 0; n < nbands; n++) {
      bind_band(equalizer_L, settings_left, n);
      bind_band(equalizer_R, settings_right, n);
    }

    // connect signals

    g_signal_connect(settings, "changed::num-bands",
                     G_CALLBACK(on_num_bands_changed), this);

    g_signal_connect(deinterleave, "pad-added",
                     G_CALLBACK(on_deinterleave_pad_added), this);

    g_signal_connect(deinterleave, "pad-removed",
                     G_CALLBACK(on_deinterleave_pad_removed), this);

    g_signal_connect(deinterleave, "no-more-pads",
                     G_CALLBACK(on_deinterleave_no_more_pads), this);

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

void Equalizer::bind_band(GstElement* equalizer,
                          GSettings* cfg,
                          const int index) {
  auto band =
      gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer), index);

  g_settings_bind(cfg,
                  std::string("band" + std::to_string(index) + "-gain").c_str(),
                  band, "gain", G_SETTINGS_BIND_GET);

  g_settings_bind(
      cfg, std::string("band" + std::to_string(index) + "-frequency").c_str(),
      band, "freq", G_SETTINGS_BIND_GET);

  g_settings_bind(
      cfg, std::string("band" + std::to_string(index) + "-width").c_str(), band,
      "bandwidth", G_SETTINGS_BIND_GET);

  g_settings_bind(cfg,
                  std::string("band" + std::to_string(index) + "-type").c_str(),
                  band, "type", G_SETTINGS_BIND_GET);

  g_object_unref(band);
}

void Equalizer::unbind_band(GstElement* equalizer, const int index) {
  auto band =
      gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer), index);

  g_settings_unbind(
      band, std::string("band" + std::to_string(index) + "-gain").c_str());

  g_settings_unbind(
      band, std::string("band" + std::to_string(index) + "-frequency").c_str());

  g_settings_unbind(
      band, std::string("band" + std::to_string(index) + "-width").c_str());

  g_settings_unbind(
      band, std::string("band" + std::to_string(index) + "-type").c_str());

  g_object_unref(band);
}

void Equalizer::update_equalizer() {
  int nbands = g_settings_get_int(settings, "num-bands");
  int current_nbands;

  g_object_get(equalizer_L, "num-bands", &current_nbands, nullptr);

  if (nbands != current_nbands) {
    util::debug(log_tag + name + ": unbinding bands");

    for (int n = 0; n < current_nbands; n++) {
      unbind_band(equalizer_L, n);
      unbind_band(equalizer_R, n);
    }

    util::debug(log_tag + name + ": setting new number of bands");

    g_object_set(equalizer_L, "num-bands", nbands, nullptr);
    g_object_set(equalizer_R, "num-bands", nbands, nullptr);

    util::debug(log_tag + name + ": binding bands");

    for (int n = 0; n < nbands; n++) {
      bind_band(equalizer_L, settings_left, n);
      bind_band(equalizer_R, settings_right, n);
    }
  }
}
