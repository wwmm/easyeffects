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

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);
    }
  });

  connect(settings, &db::EchoCanceller::filterLengthChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    int near_end_suppression = settings->nearEndSuppression();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
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

  if (echo_state_L != nullptr) {
    speex_echo_state_destroy(echo_state_L);
  }

  if (echo_state_R != nullptr) {
    speex_echo_state_destroy(echo_state_R);
  }

  free_speex();

  data_mutex.unlock();

  util::debug(log_tag + name.toStdString() + " destroyed");
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
    data_L[j] = static_cast<spx_int16_t>(left_in[j] * (SHRT_MAX + 1));
    data_R[j] = static_cast<spx_int16_t>(right_in[j] * (SHRT_MAX + 1));

    /**
     * This is a very naive and not correct attempt to mitigate the shortcomes
     * discussed at
     * https://github.com/wwmm/easyeffects/issues/1566.
     */

    probe_mono[j] = static_cast<spx_int16_t>(0.5F * (probe_left[j] + probe_right[j]) * (SHRT_MAX + 1));
  }

  speex_echo_cancellation(echo_state_L, data_L.data(), probe_mono.data(), filtered_L.data());
  speex_echo_cancellation(echo_state_R, data_R.data(), probe_mono.data(), filtered_R.data());

  speex_preprocess_run(state_left, filtered_L.data());
  speex_preprocess_run(state_right, filtered_R.data());

  for (size_t j = 0U; j < filtered_L.size(); j++) {
    left_out[j] = static_cast<float>(filtered_L[j]) * inv_short_max;

    right_out[j] = static_cast<float>(filtered_R[j]) * inv_short_max;
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

  get_peaks(left_in, right_in, left_out, right_out);
}

void EchoCanceller::init_speex() {
  if (n_samples == 0U || rate == 0U) {
    return;
  }

  data_L.resize(n_samples);
  data_R.resize(n_samples);
  probe_mono.resize(n_samples);
  filtered_L.resize(n_samples);
  filtered_R.resize(n_samples);

  int residual_echo_suppression = settings->residualEchoSuppression();
  int near_end_suppression = settings->nearEndSuppression();

  const uint filter_length = static_cast<uint>(0.001F * static_cast<float>(settings->filterLength() * rate));

  util::debug(std::format("{}{} filter length: {}", log_tag, name.toStdString(), filter_length));

  if (echo_state_L != nullptr) {
    speex_echo_state_destroy(echo_state_L);
  }

  echo_state_L = speex_echo_state_init(static_cast<int>(n_samples), static_cast<int>(filter_length));

  if (speex_echo_ctl(echo_state_L, SPEEX_ECHO_SET_SAMPLING_RATE, &rate) != 0) {
    util::warning(log_tag + name.toStdString() + "SPEEX_ECHO_SET_SAMPLING_RATE: unknown request");
  }

  if (echo_state_R != nullptr) {
    speex_echo_state_destroy(echo_state_R);
  }

  echo_state_R = speex_echo_state_init(static_cast<int>(n_samples), static_cast<int>(filter_length));

  if (speex_echo_ctl(echo_state_R, SPEEX_ECHO_SET_SAMPLING_RATE, &rate) != 0) {
    util::warning(log_tag + name.toStdString() + "SPEEX_ECHO_SET_SAMPLING_RATE: unknown request");
  }

  if (state_left != nullptr) {
    speex_preprocess_state_destroy(state_left);
  }

  if (state_right != nullptr) {
    speex_preprocess_state_destroy(state_right);
  }

  state_left = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));
  state_right = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));

  if (state_left != nullptr) {
    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state_L);

    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);

    speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
  }

  if (state_right != nullptr) {
    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state_R);

    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &residual_echo_suppression);

    speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &near_end_suppression);
  }

  ready = true;
}

void EchoCanceller::free_speex() {
  if (state_left != nullptr) {
    speex_preprocess_state_destroy(state_left);
  }

  if (state_right != nullptr) {
    speex_preprocess_state_destroy(state_right);
  }

  state_left = nullptr;
  state_right = nullptr;
}

auto EchoCanceller::get_latency_seconds() -> float {
  return latency_value;
}
