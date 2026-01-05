/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "loudness.hpp"
#include <qobject.h>
#include <sys/types.h>
#include <algorithm>
#include <format>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_loudness.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Loudness::Loudness(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::loudness,
                 tags::plugin_package::Package::lsp,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Loudness>(pipe_type,
                                                          tags::plugin_name::BaseName::loudness + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://lsp-plug.in/plugins/lv2/loud_comp_stereo";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Loudness>(settings);

  BIND_LV2_PORT("mode", mode, setMode, db::Loudness::modeChanged);
  BIND_LV2_PORT("std", std, setStd, db::Loudness::stdChanged);
  BIND_LV2_PORT("fft", fft, setFft, db::Loudness::fftChanged);
  BIND_LV2_PORT("approx", iirApproximation, setIirApproximation, db::Loudness::iirApproximationChanged);
  BIND_LV2_PORT("volume", volume, setVolume, db::Loudness::volumeChanged);
  BIND_LV2_PORT("hclip", clipping, setClipping, db::Loudness::clippingChanged);
  BIND_LV2_PORT("hcrange", clippingRange, setClippingRange, db::Loudness::clippingRangeChanged);
}

Loudness::~Loudness() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Loudness::reset() {
  settings->setDefaults();
}

void Loudness::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void Loudness::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

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

  // This plugin gives the latency in number of samples

  const auto lv = static_cast<uint>(lv2_wrapper->get_control_port_value("out_latency"));

  if (latency_n_frames != lv) {
    latency_n_frames = lv;

    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Loudness::process([[maybe_unused]] std::span<float>& left_in,
                       [[maybe_unused]] std::span<float>& right_in,
                       [[maybe_unused]] std::span<float>& left_out,
                       [[maybe_unused]] std::span<float>& right_out,
                       [[maybe_unused]] std::span<float>& probe_left,
                       [[maybe_unused]] std::span<float>& probe_right) {}

auto Loudness::get_latency_seconds() -> float {
  return latency_value;
}
