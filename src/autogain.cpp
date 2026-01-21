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

#include "autogain.hpp"
#include <ebur128.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qtypes.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <format>
#include <mutex>
#include <numbers>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_autogain.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Autogain::Autogain(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::autogain,
                 tags::plugin_package::Package::ebur128,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Autogain>(pipe_type,
                                                          tags::plugin_name::BaseName::autogain + "#" + instance_id)) {
  // bypass, input and output gain controls

  init_common_controls<db::Autogain>(settings);

  // specific plugin controls

  connect(settings, &db::Autogain::maximumHistoryChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    set_maximum_history(settings->maximumHistory());
  });
}

Autogain::~Autogain() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  ebur128_ready = false;

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);
  }

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Autogain::reset() {
  settings->setDefaults();
}

void Autogain::clear_data() {
  setup();
}

auto Autogain::init_ebur128() -> bool {
  if (n_samples == 0U || rate == 0U) {
    return false;
  }

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);

    ebur_state = nullptr;
  }

  ebur_state = ebur128_init(2U, rate, EBUR128_MODE_S | EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_SAMPLE_PEAK);

  ebur128_set_channel(ebur_state, 0U, EBUR128_LEFT);
  ebur128_set_channel(ebur_state, 1U, EBUR128_RIGHT);

  set_maximum_history(settings->maximumHistory());

  return ebur_state != nullptr;
}

void Autogain::set_maximum_history(const int& seconds) {
  if (ebur_state == nullptr) {
    return;
  }

  // The value given to ebur128_set_max_history must be in milliseconds

  ebur128_set_max_history(ebur_state, static_cast<ulong>(seconds) * 1000UL);
}

void Autogain::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  block_time = static_cast<double>(n_samples) / static_cast<double>(rate);

  attack_coeff = std::exp(-block_time / attack_time);
  release_coeff = std::exp(-block_time / release_time);

  if (2U * static_cast<size_t>(n_samples) != data.size()) {
    data.resize(static_cast<size_t>(n_samples) * 2U);
  }

  // There is no need to reset libebur128 when n_samples change.
  // Only rate changes matter for it.

  if (ebur128_ready && rate == old_rate) {
    return;
  }

  ebur128_ready = false;

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (ebur128_ready) {
          return;
        }

        auto status = true;

        old_rate = rate;

        status = init_ebur128();

        std::scoped_lock<std::mutex> lock(data_mutex);

        ebur128_ready = status;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Autogain::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  if (!ebur128_ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    const float final_gain = static_cast<float>(internal_output_gain) * output_gain;

    if (final_gain != 1.0F) {
      apply_gain(left_out, right_out, final_gain);
    }

    return;
  }

  {
    const float* __restrict__ l = left_in.data();
    const float* __restrict__ r = right_in.data();
    float* __restrict__ d = data.data();

    for (size_t i = 0; i < n_samples; ++i) {
      const size_t idx = i * 2;

      d[idx] = l[i];
      d[idx + 1] = r[i];
    }
  }

  ebur128_add_frames_float(ebur_state, data.data(), n_samples);

  auto failed = false;

  failed = (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary)) ||
           (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm)) ||
           (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global)) ||
           (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative)) ||
           (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range));

  if (std::isinf(momentary) || std::isnan(momentary)) {
    /**
     * Assuming zero so that the output gain is negative.
     * This should avoid undesirably high amplification in case
     * a bad resutla comes from libebur128
     */

    momentary = 0.0;
  }

  if (shortterm > 10.0 || std::isinf(shortterm) || std::isnan(shortterm)) {
    /**
     * Sometimes when a stream is started right after Easy Effects has been
     * initialized a very large shorterm value is calculated.
     * Probably because of some weird high intensity transient. So it is better
     * to ignore unreasonable large values. When they happen we just set the
     * shorterm value to the momentary loudness.
     */

    shortterm = momentary;
  }

  if (global > 10.0 || std::isinf(global) || std::isnan(global)) {
    /**
     * Sometimes when a stream is started right after Easy Effects has been
     * initialized a very large integrated value is calculated.
     * Probably because of some weird high intensity transient. So it is better
     * to ignore unreasonable large values. When they happen we just set the
     * global value to the momentary loudness.
     */

    global = momentary;
  }

  if (momentary > settings->silenceThreshold() && !failed) {
    double peak_L = 0.0;
    double peak_R = 0.0;

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 0U, &peak_L)) {
      failed = true;
    }

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 1U, &peak_R)) {
      failed = true;
    }

    if (!failed) {
      switch (settings->reference()) {
        case 0:  // momentary
          loudness = momentary;
          break;
        case 1:  // shortterm
          loudness = shortterm;
          break;
        case 2:  // integrated
          loudness = global;
          break;
        case 3:  // Geometric Mean (MSI)
          loudness = std::cbrt(momentary * shortterm * global);
          break;
        case 4: {  // Geometric Mean (MSI)
          loudness = std::sqrt(std::fabs(momentary * shortterm));

          if (momentary < 0 && shortterm < 0) {
            loudness *= -1;
          }

          break;
        }
        case 5: {  // Geometric Mean (MS)
          loudness = std::sqrt(std::fabs(momentary * global));

          if (momentary < 0 && global < 0) {
            loudness *= -1;
          }

          break;
        }
        case 6: {  // Geometric Mean (SI)
          loudness = std::sqrt(std::fabs(shortterm * global));

          if (shortterm < 0 && global < 0) {
            loudness *= -1;
          }

          break;
        }
        default:
          break;
      }

      const double diff = settings->target() - loudness;

      // 10^(diff/20). The way below should be faster than using pow
      const double gain = std::exp((diff / 20.0) * std::numbers::ln10);

      const double peak = (peak_L > peak_R) ? peak_L : peak_R;

      const auto db_peak = util::linear_to_db(peak);

      if (db_peak > util::minimum_db_level) {
        if (gain * peak < 1.0) {
          // Smoothing the gain correction through a leaky integrator:
          // g[n]=α⋅g[n−1]+(1−α)⋅gtarget​[n]

          // choose based on whether gain is rising or falling
          double alpha = (gain < prev_gain) ? attack_coeff : release_coeff;

          internal_output_gain = (alpha * prev_gain) + ((1.0 - alpha) * gain);

          prev_gain = internal_output_gain;
        }
      }
    }
  } else if (settings->forceSilence()) {
    internal_output_gain = util::minimum_linear_d_level;
  }

  std::ranges::copy(left_in, left_out.begin());
  std::ranges::copy(right_in, right_out.begin());

  const float final_gain = static_cast<float>(internal_output_gain) * output_gain;

  if (final_gain != 1.0F) {
    apply_gain(left_out, right_out, final_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Autogain::process([[maybe_unused]] std::span<float>& left_in,
                       [[maybe_unused]] std::span<float>& right_in,
                       [[maybe_unused]] std::span<float>& left_out,
                       [[maybe_unused]] std::span<float>& right_out,
                       [[maybe_unused]] std::span<float>& probe_left,
                       [[maybe_unused]] std::span<float>& probe_right) {}

auto Autogain::get_latency_seconds() -> float {
  return 0.0F;
}

float Autogain::getMomentaryLevel() const {
  return momentary;
}

float Autogain::getShorttermLevel() const {
  return shortterm;
}

float Autogain::getIntegratedLevel() const {
  return global;
}

float Autogain::getRelativeLevel() const {
  return relative;
}

float Autogain::getRangeLevel() const {
  return range;
}

float Autogain::getLoudnessLevel() const {
  return loudness;
}

float Autogain::getOutputGainLevel() const {
  return util::linear_to_db(internal_output_gain);
}

void Autogain::resetHistory() {
  internal_output_gain = 1.0;

  data_mutex.lock();

  ebur128_ready = false;

  data_mutex.unlock();

  setup();
}
