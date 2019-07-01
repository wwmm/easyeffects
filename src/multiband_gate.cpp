#include "multiband_gate.hpp"
#include <glibmm/main.h>
#include "util.hpp"

namespace {

void on_post_messages_changed(GSettings* settings, gchar* key, MultibandGate* l) {
  auto post = g_settings_get_boolean(settings, key);

  if (post) {
    if (!l->input_level_connection.connected()) {
      l->input_level_connection = Glib::signal_timeout().connect(
          [l]() {
            float inL, inR;

            g_object_get(l->multiband_gate, "meter-inL", &inL, nullptr);
            g_object_get(l->multiband_gate, "meter-inR", &inR, nullptr);

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

            g_object_get(l->multiband_gate, "meter-outL", &outL, nullptr);
            g_object_get(l->multiband_gate, "meter-outR", &outR, nullptr);

            std::array<double, 2> out_peak = {outL, outR};

            l->output_level.emit(out_peak);

            return true;
          },
          100);
    }

    if (!l->output0_connection.connected()) {
      l->output0_connection = Glib::signal_timeout().connect(
          [l]() {
            float output;

            g_object_get(l->multiband_gate, "output0", &output, nullptr);

            l->output0.emit(output);

            return true;
          },
          100);
    }

    if (!l->output1_connection.connected()) {
      l->output1_connection = Glib::signal_timeout().connect(
          [l]() {
            float output;

            g_object_get(l->multiband_gate, "output1", &output, nullptr);

            l->output1.emit(output);

            return true;
          },
          100);
    }

    if (!l->output2_connection.connected()) {
      l->output2_connection = Glib::signal_timeout().connect(
          [l]() {
            float output;

            g_object_get(l->multiband_gate, "output2", &output, nullptr);

            l->output2.emit(output);

            return true;
          },
          100);
    }

    if (!l->output3_connection.connected()) {
      l->output3_connection = Glib::signal_timeout().connect(
          [l]() {
            float output;

            g_object_get(l->multiband_gate, "output3", &output, nullptr);

            l->output3.emit(output);

            return true;
          },
          100);
    }

    if (!l->gating0_connection.connected()) {
      l->gating0_connection = Glib::signal_timeout().connect(
          [l]() {
            float gating;

            g_object_get(l->multiband_gate, "gating0", &gating, nullptr);

            l->gating0.emit(gating);

            return true;
          },
          100);
    }

    if (!l->gating1_connection.connected()) {
      l->gating1_connection = Glib::signal_timeout().connect(
          [l]() {
            float gating;

            g_object_get(l->multiband_gate, "gating1", &gating, nullptr);

            l->gating1.emit(gating);

            return true;
          },
          100);
    }

    if (!l->gating2_connection.connected()) {
      l->gating2_connection = Glib::signal_timeout().connect(
          [l]() {
            float gating;

            g_object_get(l->multiband_gate, "gating2", &gating, nullptr);

            l->gating2.emit(gating);

            return true;
          },
          100);
    }

    if (!l->gating3_connection.connected()) {
      l->gating3_connection = Glib::signal_timeout().connect(
          [l]() {
            float gating;

            g_object_get(l->multiband_gate, "gating3", &gating, nullptr);

            l->gating3.emit(gating);

            return true;
          },
          100);
    }
  } else {
    l->output0_connection.disconnect();
    l->output1_connection.disconnect();
    l->output2_connection.disconnect();
    l->output3_connection.disconnect();

    l->gating0_connection.disconnect();
    l->gating1_connection.disconnect();
    l->gating2_connection.disconnect();
    l->gating3_connection.disconnect();
  }
}

}  // namespace

MultibandGate::MultibandGate(const std::string& tag, const std::string& schema)
    : PluginBase(tag, "multiband_gate", schema) {
  multiband_gate = gst_element_factory_make("calf-sourceforge-net-plugins-MultibandGate", nullptr);

  if (is_installed(multiband_gate)) {
    auto audioconvert_in = gst_element_factory_make("audioconvert", "multiband_gate_audioconvert_in");
    auto audioconvert_out = gst_element_factory_make("audioconvert", "multiband_gate_audioconvert_out");

    gst_bin_add_many(GST_BIN(bin), audioconvert_in, multiband_gate, audioconvert_out, nullptr);
    gst_element_link_many(audioconvert_in, multiband_gate, audioconvert_out, nullptr);

    auto pad_sink = gst_element_get_static_pad(audioconvert_in, "sink");
    auto pad_src = gst_element_get_static_pad(audioconvert_out, "src");

    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

    gst_object_unref(GST_OBJECT(pad_sink));
    gst_object_unref(GST_OBJECT(pad_src));

    g_object_set(multiband_gate, "bypass", false, nullptr);

    bind_to_gsettings();

    g_signal_connect(settings, "changed::post-messages", G_CALLBACK(on_post_messages_changed), this);

    // useless write just to force callback call

    auto enable = g_settings_get_boolean(settings, "state");

    g_settings_set_boolean(settings, "state", enable);
  }
}

MultibandGate::~MultibandGate() {
  util::debug(log_tag + name + " destroyed");
}

void MultibandGate::bind_to_gsettings() {
  g_settings_bind_with_mapping(settings, "input-gain", multiband_gate, "level-in", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "output-gain", multiband_gate, "level-out", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "freq0", multiband_gate, "freq0", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "freq1", multiband_gate, "freq1", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "freq2", multiband_gate, "freq2", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind(settings, "mode", multiband_gate, "mode", G_SETTINGS_BIND_DEFAULT);

  // sub band

  g_settings_bind_with_mapping(settings, "range0", multiband_gate, "range0", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "threshold0", multiband_gate, "threshold0", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio0", multiband_gate, "ratio0", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "attack0", multiband_gate, "attack0", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release0", multiband_gate, "release0", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "makeup0", multiband_gate, "makeup0", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "knee0", multiband_gate, "knee0", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "detection0", multiband_gate, "detection0", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "bypass0", multiband_gate, "bypass0", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "solo0", multiband_gate, "solo0", G_SETTINGS_BIND_DEFAULT);

  // low band

  g_settings_bind_with_mapping(settings, "range1", multiband_gate, "range1", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "threshold1", multiband_gate, "threshold1", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio1", multiband_gate, "ratio1", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "attack1", multiband_gate, "attack1", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release1", multiband_gate, "release1", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "makeup1", multiband_gate, "makeup1", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "knee1", multiband_gate, "knee1", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "detection1", multiband_gate, "detection1", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "bypass1", multiband_gate, "bypass1", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "solo1", multiband_gate, "solo1", G_SETTINGS_BIND_DEFAULT);

  // mid

  g_settings_bind_with_mapping(settings, "range2", multiband_gate, "range2", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "threshold2", multiband_gate, "threshold2", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio2", multiband_gate, "ratio2", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "attack2", multiband_gate, "attack2", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release2", multiband_gate, "release2", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "makeup2", multiband_gate, "makeup2", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "knee2", multiband_gate, "knee2", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "detection2", multiband_gate, "detection2", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "bypass2", multiband_gate, "bypass2", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "solo2", multiband_gate, "solo2", G_SETTINGS_BIND_DEFAULT);

  // high band

  g_settings_bind_with_mapping(settings, "range3", multiband_gate, "range3", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "threshold3", multiband_gate, "threshold3", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "ratio3", multiband_gate, "ratio3", G_SETTINGS_BIND_GET, util::double_to_float,
                               nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "attack3", multiband_gate, "attack3", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "release3", multiband_gate, "release3", G_SETTINGS_BIND_GET,
                               util::double_to_float, nullptr, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "makeup3", multiband_gate, "makeup3", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind_with_mapping(settings, "knee3", multiband_gate, "knee3", G_SETTINGS_BIND_DEFAULT,
                               util::db20_gain_to_linear, util::linear_gain_to_db20, nullptr, nullptr);

  g_settings_bind(settings, "detection3", multiband_gate, "detection3", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "bypass3", multiband_gate, "bypass3", G_SETTINGS_BIND_DEFAULT);

  g_settings_bind(settings, "solo3", multiband_gate, "solo3", G_SETTINGS_BIND_DEFAULT);
}
