/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "autogain.hpp"
#include <ebur128.h>
#include <qtypes.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
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
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  for (auto& t : mythreads) {
    t.join();
  }

  mythreads.clear();

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);
  }

  settings->disconnect();

  util::debug(log_tag + name.toStdString() + " destroyed");
}

void Autogain::reset() {
  settings->setDefaults();
}

auto Autogain::init_ebur128() -> bool {
  if (n_samples == 0U || rate == 0U) {
    return false;
  }

  internal_output_gain = 1.0;

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
  if (2U * static_cast<size_t>(n_samples) != data.size()) {
    data.resize(static_cast<size_t>(n_samples) * 2U);
  }

  if (rate != old_rate) {
    data_mutex.lock();

    ebur128_ready = false;

    data_mutex.unlock();

    mythreads.emplace_back([this]() {  // Using emplace_back here makes sense
      if (ebur128_ready) {
        return;
      }

      auto status = true;

      old_rate = rate;

      status = init_ebur128();

      data_mutex.lock();

      ebur128_ready = status;

      data_mutex.unlock();
    });
  }
}

void Autogain::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !ebur128_ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  for (size_t n = 0U; n < n_samples; n++) {
    data[2U * n] = left_in[n];
    data[(2U * n) + 1U] = right_in[n];
  }

  ebur128_add_frames_float(ebur_state, data.data(), n_samples);

  auto failed = false;

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global)) {
    failed = true;
  }

  if (std::isinf(momentary) || std::isnan(momentary)) {
    /*
      Assuming zero so that the output gain is negative. This should avoid undesirably high amplification in case
      a bad resutla comes from libebur128
    */

    momentary = 0.0;
  }

  if (shortterm > 10.0 || std::isinf(shortterm) || std::isnan(shortterm)) {
    /*
      Sometimes when a stream is started right after Easy Effects has been initialized a very large shorterm value is
      calculated. Probably because of some weird high intensity transient. So it is better to ignore unresonable large
       values. When they happen we just set the shorterm value to the momentary loudness.
    */

    shortterm = momentary;
  }

  if (global > 10.0 || std::isinf(global) || std::isnan(global)) {
    /*
      Sometimes when a stream is started right after Easy Effects has been initialized a very large integrated value
      is calculated. Probably because of some weird high intensity transient. So it is better to ignore unresonable
      large values. When they happen we just set the global value to the momentary loudness.
    */

    global = momentary;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative)) {
    failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range)) {
    failed = true;
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
      if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Momentary") {
        loudness = momentary;
      } else if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Shortterm") {
        loudness = shortterm;
      } else if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Integrated") {
        loudness = global;
      } else if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Geometric Mean (MSI)") {
        loudness = std::cbrt(momentary * shortterm * global);
      } else if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Geometric Mean (MS)") {
        loudness = std::sqrt(std::fabs(momentary * shortterm));

        if (momentary < 0 && shortterm < 0) {
          loudness *= -1;
        }
      } else if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Geometric Mean (MI)") {
        loudness = std::sqrt(std::fabs(momentary * global));

        if (momentary < 0 && global < 0) {
          loudness *= -1;
        }
      } else if (settings->defaultReferenceLabelsValue()[settings->reference()] == "Geometric Mean (SI)") {
        loudness = std::sqrt(std::fabs(shortterm * global));

        if (shortterm < 0 && global < 0) {
          loudness *= -1;
        }
      }

      const double diff = settings->target() - loudness;

      // 10^(diff/20). The way below should be faster than using pow
      const double gain = std::exp((diff / 20.0) * std::numbers::ln10);

      const double peak = (peak_L > peak_R) ? peak_L : peak_R;

      const auto db_peak = util::linear_to_db(peak);

      if (db_peak > util::minimum_db_level) {
        if (gain * peak < 1.0) {
          internal_output_gain = gain;
        }
      }
    }
  } else if (settings->forceSilence()) {
    internal_output_gain = util::minimum_linear_d_level;
  }

  std::ranges::copy(left_in, left_out.begin());
  std::ranges::copy(right_in, right_out.begin());

  if (internal_output_gain != 1.0F) {
    apply_gain(left_out, right_out, static_cast<float>(internal_output_gain));
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  get_peaks(left_in, right_in, left_out, right_out);
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
  mythreads.emplace_back([&]() {  // Using emplace_back here makes sense
    data_mutex.lock();

    ebur128_ready = false;

    data_mutex.unlock();

    auto status = init_ebur128();

    data_mutex.lock();

    ebur128_ready = status;

    data_mutex.unlock();
  });
}
