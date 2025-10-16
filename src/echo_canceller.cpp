/**
 * Copyright © 2017-2025 Wellington Wallace
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

#include "echo_canceller.hpp"
#include <api/audio/audio_processing.h>
#include <qobject.h>
#include <sys/types.h>
#include <algorithm>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include <vector>
#include "db_manager.hpp"
#include "easyeffects_db_echo_canceller.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

EchoCanceller::EchoCanceller(const std::string& tag,
                             pw::Manager* pipe_manager,
                             PipelineType pipe_type,
                             QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::echoCanceller,
                 tags::plugin_package::Package::speex,
                 instance_id,
                 pipe_manager,
                 pipe_type,
                 true),
      settings(db::Manager::self().get_plugin_db<db::EchoCanceller>(
          pipe_type,
          tags::plugin_name::BaseName::echoCanceller + "#" + instance_id)) {
  init_common_controls<db::EchoCanceller>(settings);

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() { std::scoped_lock<std::mutex> lock(data_mutex); });

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() { std::scoped_lock<std::mutex> lock(data_mutex); });

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() { std::scoped_lock<std::mutex> lock(data_mutex); });
}

EchoCanceller::~EchoCanceller() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  data_mutex.lock();

  ready = false;

  data_mutex.unlock();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void EchoCanceller::reset() {
  settings->setDefaults();
}

void EchoCanceller::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  notify_latency = true;

  latency_n_frames = 0U;

  init_webrtc();
}

void EchoCanceller::process([[maybe_unused]] std::span<float>& left_in,
                            [[maybe_unused]] std::span<float>& right_in,
                            [[maybe_unused]] std::span<float>& left_out,
                            [[maybe_unused]] std::span<float>& right_out) {}

void EchoCanceller::process(std::span<float>& left_in,
                            std::span<float>& right_in,
                            std::span<float>& left_out,
                            std::span<float>& right_out,
                            std::span<float>& probe_left,
                            std::span<float>& probe_right) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  buf_near_L.insert(buf_near_L.end(), left_in.begin(), left_in.end());
  buf_near_R.insert(buf_near_R.end(), right_in.begin(), right_in.end());
  buf_far_L.insert(buf_far_L.end(), probe_left.begin(), probe_left.end());
  buf_far_R.insert(buf_far_R.end(), probe_right.begin(), probe_right.end());

  auto copy_bulk = [](auto& buffer, auto& channel_data) {
    std::copy_n(buffer.begin(), channel_data.size(), channel_data.begin());

    buffer.erase(buffer.begin(), buffer.begin() + channel_data.size());
  };

  while (buf_near_L.size() >= near_L.size()) {
    copy_bulk(buf_near_L, near_L);
    copy_bulk(buf_near_R, near_R);
    copy_bulk(buf_far_L, far_L);
    copy_bulk(buf_far_R, far_R);

    float* near_ptrs[2] = {near_L.data(), near_R.data()};
    float* far_ptrs[2] = {far_L.data(), far_R.data()};

    ap_builder->ProcessReverseStream(far_ptrs, stream_config, stream_config, far_ptrs);
    ap_builder->ProcessStream(near_ptrs, stream_config, stream_config, near_ptrs);

    buf_out_L.insert(buf_out_L.end(), near_L.begin(), near_L.end());
    buf_out_R.insert(buf_out_R.end(), near_R.begin(), near_R.end());
  }

  if (buf_out_L.size() >= n_samples) {
    copy_bulk(buf_out_L, left_out);
    copy_bulk(buf_out_R, right_out);
  } else {
    const uint offset = n_samples - buf_out_L.size();

    if (offset != latency_n_frames) {
      latency_n_frames = offset;

      notify_latency = true;
    }

    // Fill beginning with zeros
    std::fill_n(left_out.begin(), offset, 0.0F);
    std::fill_n(right_out.begin(), offset, 0.0F);

    std::ranges::copy(buf_out_L, left_out.begin() + offset);
    std::ranges::copy(buf_out_R, right_out.begin() + offset);

    buf_out_L.clear();
    buf_out_R.clear();
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    const float latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void EchoCanceller::init_webrtc() {
  if (n_samples == 0U || rate == 0U) {
    return;
  }

  blocksize = rate * 0.01;  // 10 ms

  util::debug(std::format("webrtc blocksize: {}", blocksize));

  near_L.resize(blocksize);
  near_R.resize(blocksize);
  far_L.resize(blocksize);
  far_R.resize(blocksize);

  buf_near_L.clear();
  buf_near_R.clear();
  buf_far_L.clear();
  buf_far_R.clear();
  buf_out_L.clear();
  buf_out_R.clear();

  webrtc::AudioProcessing::Config cfg;

  cfg.echo_canceller.enabled = true;
  cfg.echo_canceller.mobile_mode = false;

  cfg.gain_controller1.enabled = true;
  // cfg.gain_controller1.mode = webrtc::AudioProcessing::Config::GainController1::kAdaptiveAnalog;
  cfg.gain_controller1.mode = webrtc::AudioProcessing::Config::GainController1::kAdaptiveDigital;

  cfg.gain_controller2.enabled = true;

  cfg.high_pass_filter.enabled = true;

  ap_builder = webrtc::AudioProcessingBuilder().Create();

  ap_builder->ApplyConfig(cfg);

  stream_config = webrtc::StreamConfig(rate, 2);

  ready = true;
}

auto EchoCanceller::get_latency_seconds() -> float {
  return latency_value;
}
