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
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/para_equalizer_x32_lr")) {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  settings->signal_changed("input-gain").connect([=, this](auto key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](auto key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("num-bands").connect([=, this](auto key) {
    int nbands = settings->get_int(key);

    for (uint n = nbands; n < max_bands; n++) {
      // turn off unused band

      settings_left->set_enum("band" + std::to_string(n) + "-type", 0);
      settings_right->set_enum("band" + std::to_string(n) + "-type", 0);
    }
  });

  lv2_wrapper->bind_key_enum(settings, "mode", "mode");

  for (uint n = 0; n < max_bands; n++) {
    bind_band(n);
  }
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

void Equalizer::bind_band(const int& index) {
  // left channel

  lv2_wrapper->bind_key_enum(settings_left, std::string("band" + std::to_string(index) + "-type"),
                             std::string("ftl_" + std::to_string(index)));

  lv2_wrapper->bind_key_enum(settings_left, std::string("band" + std::to_string(index) + "-mode"),
                             std::string("fml_" + std::to_string(index)));

  lv2_wrapper->bind_key_enum(settings_left, std::string("band" + std::to_string(index) + "-slope"),
                             std::string("sl_" + std::to_string(index)));

  lv2_wrapper->bind_key_bool(settings_left, std::string("band" + std::to_string(index) + "-solo"),
                             std::string("xsl_" + std::to_string(index)));

  lv2_wrapper->bind_key_bool(settings_left, std::string("band" + std::to_string(index) + "-mute"),
                             std::string("xml_" + std::to_string(index)));

  lv2_wrapper->bind_key_double(settings_left, std::string("band" + std::to_string(index) + "-frequency"),
                               std::string("fl_" + std::to_string(index)));

  lv2_wrapper->bind_key_double(settings_left, std::string("band" + std::to_string(index) + "-gain"),
                               std::string("gl_" + std::to_string(index)));

  lv2_wrapper->bind_key_double_db(settings_left, std::string("band" + std::to_string(index) + "-q"),
                                  std::string("ql_" + std::to_string(index)));

  // right channel

  lv2_wrapper->bind_key_enum(settings_right, std::string("band" + std::to_string(index) + "-type"),
                             std::string("ftr_" + std::to_string(index)));

  lv2_wrapper->bind_key_enum(settings_right, std::string("band" + std::to_string(index) + "-mode"),
                             std::string("fmr_" + std::to_string(index)));

  lv2_wrapper->bind_key_enum(settings_right, std::string("band" + std::to_string(index) + "-slope"),
                             std::string("sr_" + std::to_string(index)));

  lv2_wrapper->bind_key_bool(settings_right, std::string("band" + std::to_string(index) + "-solo"),
                             std::string("xsr_" + std::to_string(index)));

  lv2_wrapper->bind_key_bool(settings_right, std::string("band" + std::to_string(index) + "-mute"),
                             std::string("xmr_" + std::to_string(index)));

  lv2_wrapper->bind_key_double(settings_right, std::string("band" + std::to_string(index) + "-frequency"),
                               std::string("fr_" + std::to_string(index)));

  lv2_wrapper->bind_key_double(settings_right, std::string("band" + std::to_string(index) + "-gain"),
                               std::string("gr_" + std::to_string(index)));

  lv2_wrapper->bind_key_double_db(settings_right, std::string("band" + std::to_string(index) + "-q"),
                                  std::string("qr_" + std::to_string(index)));

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

  //   g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-q").c_str(),
  //   equalizer,
  //                                std::string("qr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
  //                                util::double_to_float, nullptr, nullptr, nullptr);

  //   g_settings_bind_with_mapping(settings_right, std::string("band" + std::to_string(index) + "-gain").c_str(),
  //   equalizer,
  //                                std::string("gr-" + std::to_string(index)).c_str(), G_SETTINGS_BIND_GET,
  //                                util::db20_gain_to_linear, nullptr, nullptr, nullptr);
}
