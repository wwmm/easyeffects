#include "gate.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, Gate* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->gating_connection.connected()) {
      l->gating_connection = Glib::signal_timeout().connect(
          [l]() {
            float gating;

            g_object_get(l->gate, "gating", &gating, nullptr);

            l->gating.emit(gating);

            return true;
          },
          100);
    }
  } else {
    l->gating_connection.disconnect();
  }
}

}  // namespace

Gate::Gate(const std::string& tag, const std::string& schema) : PluginBase(tag, "gate", schema) {
  gate = gst_element_factory_make("calf-sourceforge-net-plugins-Gate", "gate");

  if (is_installed(gate)) {
    auto in_level = gst_element_factory_make("level", "gate_input_level");
    auto out_level = gst_element_factory_make("level", "gate_output_level");
    auto audioconvert_in = gst_element_factory_make("audioconvert", "gate_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "gate_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), in_level, audioconvert_in, gate, audioconvert_out, out_level, nullptr);
    gst_element_link_many(in_level, audioconvert_in, gate, audioconvert_out, out_level, nullptr);

    auto pad_sink = gst_element_get_static_pad(in_level, "sink");
    auto pad_src = gst_element_get_static_pad(out_level, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(gate, "bypass", false, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

Gate::~Gate() {
  util::debug(log_tag + name + " destroyed");
}

void Gate::bind_to_gsettings() {
  g_settings_bind(settings, "detection", gate, "detection", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "stereo-link", gate, "stereo-link", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind_with_mapping(settings, "range", gate, "range", G_SETTINGS_BIND_GET, util::db20_gain_to_linear,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "attack", gate, "attack", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release", gate, "release", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio", gate, "ratio", G_SETTINGS_BIND_GET, util::double_to_float, nullptr,
                               nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "threshold", gate, "threshold", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "makeup", gate, "makeup", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "knee", gate, "knee", G_SETTINGS_BIND_DEFAULT, util::db20_gain_to_linear,
                               util::linear_gain_to_db20, nullptr, nullptr);
}
