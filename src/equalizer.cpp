/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "equalizer.hpp"

// namespace {

// void on_num_bands_changed(GSettings* settings, gchar* key, Equalizer* l) {
//   l->update_equalizer();
// }

// }  // namespace

Equalizer::Equalizer(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_path,
                     const std::string& schema_channel,
                     const std::string& schema_channel_left_path,
                     const std::string& schema_channel_right_path,
                     PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::equalizer, schema, schema_path, pipe_manager),
      settings_left(Gio::Settings::create(schema_channel.c_str(), schema_channel_left_path.c_str())),
      settings_right(Gio::Settings::create(schema_channel.c_str(), schema_channel_right_path.c_str())),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/Filter")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  // equalizer = gst_element_factory_make("lsp-plug-in-plugins-lv2-para-equalizer-x32-lr", nullptr);

  // if (is_installed(equalizer)) {
  //   auto* input_gain = gst_element_factory_make("volume", nullptr);
  //   auto* in_level = gst_element_factory_make("level", "equalizer_input_level");
  //   auto* out_level = gst_element_factory_make("level", "equalizer_output_level");
  //   auto* output_gain = gst_element_factory_make("volume", nullptr);

  //   auto* audioconvert_in = gst_element_factory_make("audioconvert", "eq_audioconvert_in");
  //   auto* audioconvert_out = gst_element_factory_make("audioconvert", "eq_audioconvert_out");

  //   gst_bin_add_many(GST_BIN(bin), input_gain, in_level, audioconvert_in, equalizer, audioconvert_out, output_gain,
  //                    out_level, nullptr);

  //   gst_element_link_many(input_gain, in_level, audioconvert_in, equalizer, audioconvert_out, output_gain, out_level,
  //                         nullptr);

  //   // setting bin ghost pads

  //   auto* pad_sink = gst_element_get_static_pad(input_gain, "sink");
  //   auto* pad_src = gst_element_get_static_pad(out_level, "src");

  //   gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad_sink));
  //   gst_element_add_pad(bin, gst_ghost_pad_new("src", pad_src));

  //   gst_object_unref(GST_OBJECT(pad_sink));
  //   gst_object_unref(GST_OBJECT(pad_src));

  //   // init

  //   g_object_set(equalizer, "enabled", 1, nullptr);
  //   g_object_set(equalizer, "bal", 0.0F, nullptr);
  //   g_object_set(equalizer, "fft", 0, nullptr);  // off

  //   for (int n = 0; n < 30; n++) {
  //     bind_band(equalizer, n);
  //   }

  //   // connect signals

  //   g_signal_connect(settings, "changed::num-bands", G_CALLBACK(on_num_bands_changed), this);

  //   // gsettings bindings

  //   g_settings_bind(settings, "post-messages", in_level, "post-messages", G_SETTINGS_BIND_DEFAULT);
  //   g_settings_bind(settings, "post-messages", out_level, "post-messages", G_SETTINGS_BIND_DEFAULT);

  //   g_settings_bind_with_mapping(settings, "input-gain", input_gain, "volume", G_SETTINGS_BIND_DEFAULT,
  //                                util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr,
  //                                nullptr);

  //   g_settings_bind_with_mapping(settings, "output-gain", output_gain, "volume", G_SETTINGS_BIND_DEFAULT,
  //                                util::db20_gain_to_linear_double, util::linear_double_gain_to_db20, nullptr,
  //                                nullptr);

  //   g_settings_bind(settings, "mode", equalizer, "mode", G_SETTINGS_BIND_DEFAULT);

  //   // useless write just to force on_state_changed callback call

  //   auto enable = g_settings_get_boolean(settings, "state");

  //   g_settings_set_boolean(settings, "state", enable);
  // }
}

Equalizer::~Equalizer() {
  util::debug(log_tag + name + " destroyed");

  pw_thread_loop_lock(pm->thread_loop);

  pw_filter_set_active(filter, false);

  pw_filter_disconnect(filter);

  pw_core_sync(pm->core, PW_ID_CORE, 0);

  pw_thread_loop_wait(pm->thread_loop);

  pw_thread_loop_unlock(pm->thread_loop);
}

void Equalizer::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->create_instance(rate);
}

void Equalizer::process(std::span<float>& left_in,
                        std::span<float>& right_in,
                        std::span<float>& left_out,
                        std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  if (lv2_wrapper->get_n_samples() != left_in.size()) {
    lv2_wrapper->set_n_samples(left_in.size());
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);

  lv2_wrapper->run();

  apply_gain(left_out, right_out, output_gain);

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}

// void Equalizer::bind_band(GstElement* equalizer, const int& index) {
//   // left channel

//   g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-type").c_str(), equalizer,
//                   std::string("ftl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-mode").c_str(), equalizer,
//                   std::string("fml-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-slope").c_str(), equalizer,
//                   std::string("sl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-solo").c_str(), equalizer,
//                   std::string("xsl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_left, std::string("band" + std::to_string(index) + "-mute").c_str(), equalizer,
//                   std::string("xml-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind_with_mapping(settings_left, std::string("band" + std::to_string(index) + "-frequency").c_str(),
//                                equalizer, std::string("fl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
//                                util::double_to_float, nullptr, nullptr, nullptr);

//   g_settings_bind_with_mapping(settings_left, std::string("band" + std::to_string(index) + "-q").c_str(), equalizer,
//                                std::string("ql-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
//                                util::double_to_float, nullptr, nullptr, nullptr);

//   g_settings_bind_with_mapping(settings_left, std::string("band" + std::to_string(index) + "-gain").c_str(),
//   equalizer,
//                                std::string("gl-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
//                                util::db20_gain_to_linear, nullptr, nullptr, nullptr);

//   // right channel

//   g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-type").c_str(), equalizer,
//                   std::string("ftr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-mode").c_str(), equalizer,
//                   std::string("fmr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-slope").c_str(), equalizer,
//                   std::string("sr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-solo").c_str(), equalizer,
//                   std::string("xsr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind(settings_right, std::string("band" + std::to_string(index) + "-mute").c_str(), equalizer,
//                   std::string("xmr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_DEFAULT);

//   g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-frequency").c_str(),
//                                equalizer, std::string("fr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
//                                util::double_to_float, nullptr, nullptr, nullptr);

//   g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-q").c_str(), equalizer,
//                                std::string("qr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
//                                util::double_to_float, nullptr, nullptr, nullptr);

//   g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-gain").c_str(),
//   equalizer,
//                                std::string("gr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
//                                util::db20_gain_to_linear, nullptr, nullptr, nullptr);
// }

void Equalizer::update_equalizer() {
  // int nbands = g_settings_get_int(settings, "num-bands");

  // for (int n = nbands; n < 30; n++) {
  //   // turn off unused band

  //   g_object_set(equalizer, std::string("ftl-" + std::to_string(n)).c_str(), 0, nullptr);

  //   g_object_set(equalizer, std::string("ftr-" + std::to_string(n)).c_str(), 0, nullptr);
  // }
}
