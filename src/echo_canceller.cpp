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
                 tags::plugin_package::Package::webrtc,
                 instance_id,
                 pipe_manager,
                 pipe_type,
                 true),
      settings(db::Manager::self().get_plugin_db<db::EchoCanceller>(
          pipe_type,
          tags::plugin_name::BaseName::echoCanceller + "#" + instance_id)) {
  init_common_controls<db::EchoCanceller>(settings);

  ap_cfg.pipeline.multi_channel_render = true;
  ap_cfg.pipeline.multi_channel_capture = true;

  ap_cfg.high_pass_filter.enabled = settings->enableHighPassFilter();
  ap_cfg.high_pass_filter.apply_in_full_band = settings->highPassFilterFullBand();

  ap_cfg.echo_canceller.enabled = settings->enableEchoCanceller();
  ap_cfg.echo_canceller.mobile_mode = settings->echoCancellerMobileMode();
  ap_cfg.echo_canceller.enforce_high_pass_filtering = settings->echoCancellerEnforceHighPass();

  ap_cfg.noise_suppression.enabled = settings->enableNoiseSuppression();
  ap_cfg.noise_suppression.level =
      static_cast<webrtc::AudioProcessing::Config::NoiseSuppression::Level>(settings->noiseSuppressionLevel());

  ap_cfg.gain_controller1.enabled = settings->enableAGC();

  // Echo Canceller

  connect(settings, &db::EchoCanceller::enableEchoCancellerChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.echo_canceller.enabled = settings->enableEchoCanceller();

    ap_builder->ApplyConfig(ap_cfg);
  });

  connect(settings, &db::EchoCanceller::echoCancellerMobileModeChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.echo_canceller.mobile_mode = settings->echoCancellerMobileMode();

    ap_builder->ApplyConfig(ap_cfg);
  });

  connect(settings, &db::EchoCanceller::echoCancellerEnforceHighPassChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.echo_canceller.enforce_high_pass_filtering = settings->echoCancellerEnforceHighPass();

    ap_builder->ApplyConfig(ap_cfg);
  });

  // Noise Suppression

  connect(settings, &db::EchoCanceller::enableNoiseSuppressionChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.noise_suppression.enabled = settings->enableNoiseSuppression();

    ap_builder->ApplyConfig(ap_cfg);
  });

  connect(settings, &db::EchoCanceller::noiseSuppressionLevelChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.noise_suppression.level =
        static_cast<webrtc::AudioProcessing::Config::NoiseSuppression::Level>(settings->noiseSuppressionLevel());

    ap_builder->ApplyConfig(ap_cfg);
  });

  // High-pass Filter

  connect(settings, &db::EchoCanceller::enableHighPassFilterChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.high_pass_filter.enabled = settings->enableHighPassFilter();

    ap_builder->ApplyConfig(ap_cfg);
  });

  connect(settings, &db::EchoCanceller::highPassFilterFullBandChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.high_pass_filter.apply_in_full_band = settings->highPassFilterFullBand();

    ap_builder->ApplyConfig(ap_cfg);
  });

  // Automatic gain control

  connect(settings, &db::EchoCanceller::enableAGCChanged, [&]() {
    if (!ap_builder) {
      return;
    }

    std::scoped_lock<std::mutex> lock(data_mutex);

    ap_cfg.gain_controller1.enabled = settings->enableAGC();

    ap_builder->ApplyConfig(ap_cfg);
  });
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

void EchoCanceller::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void EchoCanceller::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

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

  while (buf_near_L.size() >= near_L.size()) {
    util::copy_bulk(buf_near_L, near_L);
    util::copy_bulk(buf_near_R, near_R);
    util::copy_bulk(buf_far_L, far_L);
    util::copy_bulk(buf_far_R, far_R);

    float* near_ptrs[2] = {near_L.data(), near_R.data()};
    float* far_ptrs[2] = {far_L.data(), far_R.data()};

    ap_builder->ProcessReverseStream(far_ptrs, stream_config, stream_config, far_ptrs);
    ap_builder->ProcessStream(near_ptrs, stream_config, stream_config, near_ptrs);

    buf_out_L.insert(buf_out_L.end(), near_L.begin(), near_L.end());
    buf_out_R.insert(buf_out_R.end(), near_R.begin(), near_R.end());
  }

  if (buf_out_L.size() >= n_samples) {
    util::copy_bulk(buf_out_L, left_out);
    util::copy_bulk(buf_out_R, right_out);
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

  blocksize = rate * 0.01;  // webrtc needs blocks of 10 ms

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

  ap_builder = webrtc::AudioProcessingBuilder().Create();

  ap_builder->ApplyConfig(ap_cfg);

  stream_config = webrtc::StreamConfig(rate, 2);

  ready = true;
}

auto EchoCanceller::get_latency_seconds() -> float {
  return latency_value;
}
