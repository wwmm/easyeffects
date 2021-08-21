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

#include "limiter.hpp"

Limiter::Limiter(const std::string& tag,
                 const std::string& schema,
                 const std::string& schema_path,
                 PipeManager* pipe_manager)
    : PluginBase(tag, plugin_name::limiter, schema, schema_path, pipe_manager),
      lv2_wrapper(std::make_unique<lv2::Lv2Wrapper>("http://lsp-plug.in/plugins/lv2/limiter_stereo")) {
  if (!lv2_wrapper->found_plugin) {
    util::debug(log_tag + "http://lsp-plug.in/plugins/lv2/limiter_stereo is not installed");
  }

  input_gain = static_cast<float>(util::db_to_linear(settings->get_double("input-gain")));
  output_gain = static_cast<float>(util::db_to_linear(settings->get_double("output-gain")));

  settings->signal_changed("input-gain").connect([=, this](const auto& key) {
    input_gain = util::db_to_linear(settings->get_double(key));
  });

  settings->signal_changed("output-gain").connect([=, this](const auto& key) {
    output_gain = util::db_to_linear(settings->get_double(key));
  });

  lv2_wrapper->bind_key_enum(settings, "mode", "mode");

  lv2_wrapper->bind_key_enum(settings, "oversampling", "ovs");

  lv2_wrapper->bind_key_enum(settings, "dithering", "dith");

  lv2_wrapper->bind_key_double(settings, "lookahead", "lk");

  lv2_wrapper->bind_key_double(settings, "attack", "at");

  lv2_wrapper->bind_key_double(settings, "release", "rt");

  lv2_wrapper->bind_key_double_db(settings, "threshold", "th");

  lv2_wrapper->bind_key_bool(settings, "gain-boost", "boost");

  lv2_wrapper->bind_key_double_db(settings, "sidechain-preamp", "scp");

  lv2_wrapper->bind_key_double(settings, "stereo-link", "slink");

  lv2_wrapper->bind_key_bool(settings, "alr", "alr");

  lv2_wrapper->bind_key_double(settings, "alr-attack", "alr_at");

  lv2_wrapper->bind_key_double(settings, "alr-release", "alr_rt");

  lv2_wrapper->bind_key_double_db(settings, "alr-knee", "knee");
}

Limiter::~Limiter() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name + " destroyed");
}

void Limiter::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);
  lv2_wrapper->create_instance(rate);
}

void Limiter::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

    return;
  }

  apply_gain(left_in, right_in, input_gain);

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  apply_gain(left_out, right_out, output_gain);

  /*
   This plugin gives the latency in number of samples
 */

  uint lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(log_tag + name + " latency: " + std::to_string(latency_value) + " s");

    Glib::signal_idle().connect_once([=, this] { latency.emit(latency_value); });

    spa_process_latency_info latency_info{};

    latency_info.ns = static_cast<uint64_t>(latency_value * 1000000000.0F);

    std::array<char, 1024> buffer{};

    spa_pod_builder b{};

    spa_pod_builder_init(&b, buffer.data(), sizeof(buffer));

    const spa_pod* param = spa_process_latency_build(&b, SPA_PARAM_ProcessLatency, &latency_info);

    pw_filter_update_params(filter, nullptr, &param, 1);
  }

  if (post_messages) {
    get_peaks(left_in, right_in, left_out, right_out);

    notification_dt += sample_duration;

    if (notification_dt >= notification_time_window) {
      float gain_l = lv2_wrapper->get_control_port_value("grlm_l");
      float gain_r = lv2_wrapper->get_control_port_value("grlm_r");
      float sidechain_l = lv2_wrapper->get_control_port_value("sclm_l");
      float sidechain_r = lv2_wrapper->get_control_port_value("sclm_r");

      Glib::signal_idle().connect_once([=, this] { gain_left.emit(gain_l); });
      Glib::signal_idle().connect_once([=, this] { gain_right.emit(gain_r); });
      Glib::signal_idle().connect_once([=, this] { sidechain_left.emit(sidechain_l); });
      Glib::signal_idle().connect_once([=, this] { sidechain_right.emit(sidechain_r); });

      notify();

      notification_dt = 0.0F;
    }
  }
}
