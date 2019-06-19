#include "reverb.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Reverb* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL, inR;

            g_object_get(l->reverb, "meter-inL", &inL, nullptr);
            g_object_get(l->reverb, "meter-inR", &inR, nullptr);

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

            g_object_get(l->reverb, "meter-outL", &outL, nullptr);
            g_object_get(l->reverb, "meter-outR", &outR, nullptr);

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

Reverb::Reverb(const std::string& tag, const std::string& schema) : PluginBase(tag, "reverb", schema) {
  reverb = gst_element_factory_make("calf-sourceforge-net-plugins-Reverb", "reverb");

  if (is_installed(reverb)) {
    auto audioconvert_in = gst_element_factory_make("audioconvert", "reverb_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "reverb_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, reverb, audioconvert_out, nullptr);

    gst_element_link_many(audioconvert_in, reverb, audioconvert_out, nullptr);

    auto pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto pad_src = gst_element_get_static_pad(audioconvert_out, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(reverb, "on", true, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Reverb::~Reverb() {
  util::debug(log_tag + name + " destroyed");
}

void Reverb::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", reverb, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", reverb, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "room-size", reverb, "room-size", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "decay-time", reverb, "decay-time", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "hf-damp", reverb, "hf-damp", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "diffusion", reverb, "diffusion", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "amount", reverb, "amount", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "dry", reverb, "dry", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "predelay", reverb, "predelay", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "bass-cut", reverb, "bass-cut", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "treble-cut", reverb, "treble-cut", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);
}
