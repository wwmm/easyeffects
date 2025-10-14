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
#include <qobject.h>
#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>
#include <speex/speexdsp_config_types.h>
#include <sys/types.h>
#include <algorithm>
#include <climits>
#include <cstddef>
#include <format>
#include <mutex>
#include <span>
#include <string>
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

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    init_speex();
  });

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    int residual_echo_suppression = settings->residualEchoSuppression();

    if (state[0]) {
      speex_preprocess_ctl(state[0], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);
    }

    if (state[1]) {
      speex_preprocess_ctl(state[1], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);
    }
  });

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    int near_end_suppression = settings->nearEndSuppression();

    if (state[0]) {
      speex_preprocess_ctl(state[0], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
    }

    if (state[1]) {
      speex_preprocess_ctl(state[1], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
    }
  });
}

EchoCanceller::~EchoCanceller() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  data_mutex.lock();

  ready = false;

  if (echo_state != nullptr) {
    speex_echo_state_destroy(echo_state);
  }

  free_speex();

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

  init_speex();
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

  for (size_t j = 0U; j < left_in.size(); j++) {
    data[j * 2U] = static_cast<spx_int16_t>(left_in[j] * (SHRT_MAX + 1));
    data[(j * 2U) + 1U] = static_cast<spx_int16_t>(right_in[j] * (SHRT_MAX + 1));

    probe[j * 2U] = static_cast<spx_int16_t>(probe_left[j] * (SHRT_MAX + 1));
    probe[(j * 2U) + 1U] = static_cast<spx_int16_t>(probe_right[j] * (SHRT_MAX + 1));
  }

  speex_echo_cancellation(echo_state, data.data(), probe.data(), filtered.data());

  // speex_preprocess_run(state, filtered.data());

  // Apply pre-processing to each channel separately
  for (size_t ch = 0; ch < 2; ch++) {
    for (size_t j = 0U; j < n_samples; j++) {
      channel[j] = filtered[(j * 2) + ch];
    }

    // Run pre-processor on this channel
    if (state[ch] != nullptr) {
      speex_preprocess_run(state[ch], channel.data());
    }

    // Put back processed data
    for (size_t j = 0U; j < n_samples; j++) {
      filtered[(j * 2) + ch] = channel[j];
    }
  }

  for (size_t j = 0U; j < left_out.size(); j++) {
    left_out[j] = static_cast<float>(filtered[j * 2U]) * inv_short_max;

    right_out[j] = static_cast<float>(filtered[(j * 2U) + 1]) * inv_short_max;
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

void EchoCanceller::init_speex() {
  if (n_samples == 0U || rate == 0U) {
    return;
  }

  if (data.size() != static_cast<size_t>(n_samples) * 2) {
    data.resize(2U * static_cast<size_t>(n_samples));
  }

  if (probe.size() != static_cast<size_t>(n_samples) * 2) {
    probe.resize(2U * static_cast<size_t>(n_samples));
  }

  if (filtered.size() != static_cast<size_t>(n_samples) * 2) {
    filtered.resize(2U * static_cast<size_t>(n_samples));
  }

  if (channel.size() != n_samples) {
    channel.resize(n_samples);
  }

  int residual_echo_suppression = settings->residualEchoSuppression();
  int near_end_suppression = settings->nearEndSuppression();

  const uint filter_length = static_cast<uint>(0.001F * static_cast<float>(settings->filterLength() * rate));

  util::debug(std::format("{}{} filter length: {}", log_tag, name.toStdString(), filter_length));

  if (echo_state != nullptr) {
    speex_echo_state_destroy(echo_state);
  }

  echo_state = speex_echo_state_init_mc(static_cast<int>(n_samples), static_cast<int>(filter_length), 2, 2);

  if (speex_echo_ctl(echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &rate) != 0) {
    util::warning(std::format("{}{}SPEEX_ECHO_SET_SAMPLING_RATE: unknown request", log_tag, name.toStdString()));
  }

  state[0] = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));
  state[1] = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));

  if (state[0] != nullptr) {
    speex_preprocess_ctl(state[0], SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state);

    speex_preprocess_ctl(state[0], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);

    speex_preprocess_ctl(state[0], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
  }

  if (state[1] != nullptr) {
    speex_preprocess_ctl(state[1], SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state);

    speex_preprocess_ctl(state[1], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);

    speex_preprocess_ctl(state[1], SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
  }

  ready = true;
}

void EchoCanceller::free_speex() {
  if (state[0] != nullptr) {
    speex_preprocess_state_destroy(state[0]);
  }

  if (state[1] != nullptr) {
    speex_preprocess_state_destroy(state[1]);
  }

  state[0] = nullptr;
  state[1] = nullptr;
}

auto EchoCanceller::get_latency_seconds() -> float {
  return latency_value;
}
