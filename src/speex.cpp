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

#include "speex.hpp"
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <speex/speex_preprocess.h>
#include <speex/speexdsp_config_types.h>
#include <QApplication>
#include <algorithm>
#include <climits>
#include <cstddef>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_speex.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Speex::Speex(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::speex,
                 tags::plugin_package::Package::speex,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::Speex>(pipe_type,
                                                            tags::plugin_name::BaseName::speex + "#" + instance_id)),
      enable_denoise(settings->enableDenoise()),
      noise_suppression(settings->noiseSuppression()),
      enable_agc(settings->enableAgc()),
      enable_vad(settings->enableVad()),
      vad_probability_start(settings->vadProbabilityStart()),
      vad_probability_continue(settings->vadProbabilityContinue()),
      enable_dereverb(settings->enableDereverb()) {
  init_common_controls<db::Speex>(settings);

  // specific plugin controls

  connect(settings, &db::Speex::enableDenoiseChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    enable_denoise = settings->enableDenoise();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_DENOISE, &enable_denoise);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_DENOISE, &enable_denoise);
    }
  });

  connect(settings, &db::Speex::noiseSuppressionChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    noise_suppression = settings->noiseSuppression();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppression);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppression);
    }
  });

  connect(settings, &db::Speex::enableAgcChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    enable_agc = settings->enableAgc();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_AGC, &enable_agc);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_AGC, &enable_agc);
    }
  });

  connect(settings, &db::Speex::enableVadChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    enable_vad = settings->enableVad();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_VAD, &enable_vad);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_VAD, &enable_vad);
    }
  });

  connect(settings, &db::Speex::vadProbabilityStartChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    vad_probability_start = settings->vadProbabilityStart();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_PROB_START, &vad_probability_start);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_PROB_START, &vad_probability_start);
    }
  });

  connect(settings, &db::Speex::vadProbabilityContinueChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    vad_probability_continue = settings->vadProbabilityContinue();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad_probability_continue);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad_probability_continue);
    }
  });

  connect(settings, &db::Speex::enableDereverbChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    enable_dereverb = settings->enableDereverb();

    if (state_left) {
      speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_DEREVERB, &enable_dereverb);
    }

    if (state_right) {
      speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_DEREVERB, &enable_dereverb);
    }
  });
}

Speex::~Speex() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  std::scoped_lock<std::mutex> lock(data_mutex);

  free_speex();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Speex::reset() {
  settings->setDefaults();
}

void Speex::clear_data() {
  setup();
}

void Speex::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  latency_n_frames = 0U;

  speex_ready = false;

  data_L.resize(n_samples);
  data_R.resize(n_samples);

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (state_left != nullptr) {
          speex_preprocess_state_destroy(state_left);
        }

        if (state_right != nullptr) {
          speex_preprocess_state_destroy(state_right);
        }

        state_left = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));
        state_right = speex_preprocess_state_init(static_cast<int>(n_samples), static_cast<int>(rate));

        if (state_left != nullptr) {
          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_DENOISE, &enable_denoise);
          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppression);

          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_AGC, &enable_agc);

          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_VAD, &enable_vad);
          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_PROB_START, &vad_probability_start);
          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad_probability_continue);

          speex_preprocess_ctl(state_left, SPEEX_PREPROCESS_SET_DEREVERB, &enable_dereverb);
        }

        if (state_right != nullptr) {
          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_DENOISE, &enable_denoise);
          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppression);

          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_AGC, &enable_agc);

          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_VAD, &enable_vad);
          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_PROB_START, &vad_probability_start);
          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vad_probability_continue);

          speex_preprocess_ctl(state_right, SPEEX_PREPROCESS_SET_DEREVERB, &enable_dereverb);
        }

        std::scoped_lock<std::mutex> lock(data_mutex);

        speex_ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Speex::process(std::span<float>& left_in,
                    std::span<float>& right_in,
                    std::span<float>& left_out,
                    std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !speex_ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  for (size_t i = 0; i < n_samples; i++) {
    data_L[i] = static_cast<spx_int16_t>(left_in[i] * (SHRT_MAX + 1));

    data_R[i] = static_cast<spx_int16_t>(right_in[i] * (SHRT_MAX + 1));
  }

  if (speex_preprocess_run(state_left, data_L.data()) == 1) {
    for (size_t i = 0; i < n_samples; i++) {
      left_out[i] = static_cast<float>(data_L[i]) * inv_short_max;
    }
  } else {
    std::ranges::fill(left_out, 0.0F);
  }

  if (speex_preprocess_run(state_right, data_R.data()) == 1) {
    for (size_t i = 0; i < n_samples; i++) {
      right_out[i] = static_cast<float>(data_R[i]) * inv_short_max;
    }
  } else {
    std::ranges::fill(right_out, 0.0F);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Speex::process([[maybe_unused]] std::span<float>& left_in,
                    [[maybe_unused]] std::span<float>& right_in,
                    [[maybe_unused]] std::span<float>& left_out,
                    [[maybe_unused]] std::span<float>& right_out,
                    [[maybe_unused]] std::span<float>& probe_left,
                    [[maybe_unused]] std::span<float>& probe_right) {}

void Speex::free_speex() {
  if (state_left != nullptr) {
    speex_preprocess_state_destroy(state_left);
  }

  if (state_right != nullptr) {
    speex_preprocess_state_destroy(state_right);
  }

  state_left = nullptr;
  state_right = nullptr;
}

auto Speex::get_latency_seconds() -> float {
  return latency_value;
}
