#include "filter.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Filter* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL, inR;

            g_object_get(l->filter, "meter-inL", &inL, nullptr);
            g_object_get(l->filter, "meter-inR", &inR, nullptr);

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

            g_object_get(l->filter, "meter-outL", &outL, nullptr);
            g_object_get(l->filter, "meter-outR", &outR, nullptr);

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

Filter::Filter(const std::string& tag, const std::string& schema) : PluginBase(tag, "filter", schema) {
  filter = gst_element_factory_make("calf-sourceforge-net-plugins-Filter", "filter");

  if (is_installed(filter)) {
    auto audioconvert_in = gst_element_factory_make("audioconvert", "filter_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "filter_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, filter, audioconvert_out, nullptr);

    gst_element_link_many(audioconvert_in, filter, audioconvert_out, nullptr);

    auto pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto pad_src = gst_element_get_static_pad(audioconvert_out, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(filter, "bypass", false, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Filter::~Filter() {
  util::debug(log_tag + name + " destroyed");
}

void Filter::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", filter, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", filter, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "frequency", filter, "freq", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "resonance", filter, "res", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "mode", filter, "mode", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "inertia", filter, "inertia", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);
}
