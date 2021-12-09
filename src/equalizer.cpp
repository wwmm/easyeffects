/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "equalizer.hpp"

using namespace std::string_literals;

Equalizer::Equalizer(const std::string& tag,
                     const std::string& schema,
                     const std::string& schema_path,
                     const std::string& schema_channel,
                     const std::string& schema_channel_left_path,
                     const std::string& schema_channel_right_path,
                     PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::equalizer, schema, schema_path, pipe_manager),
      settings_left(g_settings_new_with_path(schema_channel.c_str(), schema_channel_left_path.c_str())),
      settings_right(g_settings_new_with_path(schema_channel.c_str(), schema_channel_right_path.c_str())),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/para_equalizer_x32_lr")) {
  if (!lv2_wrapper->found_plugin) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/para_equalizer_x32_lr is not installed");
  }

  lv2_wrapper->bind_key_enum<"mode", "mode">(settings);

  bind_bands(std::make_index_sequence<max_bands>());

  on_split_channels();

  gconnections.push_back(g_signal_connect(settings, "changed::num-bands",
                                          G_CALLBACK(+[](GSettings* settings, char* key, gpointer user_data) {
                                            auto self = static_cast<Equalizer*>(user_data);

                                            const uint nbands = g_settings_get_int(settings, key);

                                            const bool split = g_settings_get_boolean(settings, "split-channels") == 1;

                                            using namespace tags::equalizer;

                                            for (uint n = 0U; n < self->max_bands; n++) {
                                              if (n < nbands) {
                                                g_settings_set_enum(self->settings_left, band_type[n], 1);

                                                if (split) {
                                                  g_settings_set_enum(self->settings_right, band_type[n], 1);
                                                }
                                              } else {
                                                // turn off unused bands
                                                g_settings_set_enum(self->settings_left, band_type[n], 0);

                                                if (split) {
                                                  g_settings_set_enum(self->settings_right, band_type[n], 0);
                                                }
                                              }
                                            }
                                          }),
                                          this));

  gconnections.push_back(g_signal_connect(
      settings, "changed::split-channels",
      G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) { self->on_split_channels(); }), this));

  setup_input_output_gain();
}

Equalizer::~Equalizer() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  for (auto& handler_id : this->gconnections_unified) {
    g_signal_handler_disconnect(this->settings_left, handler_id);
  }

  this->gconnections_unified.clear();

  util::debug(log_tag + name + " destroyed");
}

void Equalizer::on_split_channels() {
  if (g_settings_get_boolean(settings, "split-channels") == 1) {
    for (auto& handler_id : gconnections_unified) {
      g_signal_handler_disconnect(settings_left, handler_id);
    }

    gconnections_unified.clear();

    return;
  }

  using namespace tags::equalizer;

  for (uint n = 0U; n < max_bands; n++) {
    g_settings_set_enum(settings_right, band_type[n], g_settings_get_enum(settings_left, band_type[n]));

    g_settings_set_enum(settings_right, band_mode[n], g_settings_get_enum(settings_left, band_mode[n]));

    g_settings_set_enum(settings_right, band_slope[n], g_settings_get_enum(settings_left, band_slope[n]));

    g_settings_set_boolean(settings_right, band_solo[n], g_settings_get_boolean(settings_left, band_solo[n]));

    g_settings_set_boolean(settings_right, band_mute[n], g_settings_get_boolean(settings_left, band_mute[n]));

    g_settings_set_double(settings_right, band_frequency[n], g_settings_get_double(settings_left, band_frequency[n]));

    g_settings_set_double(settings_right, band_gain[n], g_settings_get_double(settings_left, band_gain[n]));

    g_settings_set_double(settings_right, band_q[n], g_settings_get_double(settings_left, band_q[n]));

    /*
      When in unified mode we want settings applied to the left channel to be propagated to the right channel
      database
    */

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_gain[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_double(self->settings_right, key,
                                                                            g_settings_get_double(settings, key));
                                                    }),
                                                    this));

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_frequency[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_double(self->settings_right, key,
                                                                            g_settings_get_double(settings, key));
                                                    }),
                                                    this));

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_q[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_double(self->settings_right, key,
                                                                            g_settings_get_double(settings, key));
                                                    }),
                                                    this));

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_type[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_enum(self->settings_right, key,
                                                                          g_settings_get_enum(settings, key));
                                                    }),
                                                    this));

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_mode[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_enum(self->settings_right, key,
                                                                          g_settings_get_enum(settings, key));
                                                    }),
                                                    this));

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_slope[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_enum(self->settings_right, key,
                                                                          g_settings_get_enum(settings, key));
                                                    }),
                                                    this));

    gconnections_unified.push_back(g_signal_connect(settings_left, ("changed::"s + band_mute[n]).c_str(),
                                                    G_CALLBACK(+[](GSettings* settings, char* key, Equalizer* self) {
                                                      g_settings_set_boolean(self->settings_right, key,
                                                                             g_settings_get_boolean(settings, key));
                                                    }),
                                                    this));
  }
}

void Equalizer::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
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

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  /*
    This plugin gives the latency in number of samples
  */

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_port_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + std::to_string(latency_port_value) + " s");

    util::idle_add([=, this]() {
      if (!post_messages) {
        return;
      }

      latency.emit(latency_port_value);
    });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_port_value * 1000000000.0F);

    std::array<char, 1024U> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += buffer_duration;

    if (notification_dt >= notification_time_window) {
      notify();

      notification_dt = 0.0F;
    }
  }
}
