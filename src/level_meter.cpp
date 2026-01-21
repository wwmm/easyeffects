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

#include "level_meter.hpp"
#include <ebur128.h>
#include <qnamespace.h>
#include <qobject.h>
#include <algorithm>
#include <cstddef>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_level_meter.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

LevelMeter::LevelMeter(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::levelMeter,
                 tags::plugin_package::Package::ebur128,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::LevelMeter>(
          pipe_type,
          tags::plugin_name::BaseName::levelMeter + "#" + instance_id)) {
  bypass = settings->bypass();

  connect(settings, &db::LevelMeter::bypassChanged, [&]() { bypass = settings->bypass(); });
}

LevelMeter::~LevelMeter() {
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

void LevelMeter::reset() {
  settings->setDefaults();
}

void LevelMeter::clear_data() {
  setup();
}

auto LevelMeter::init_ebur128() -> bool {
  if (n_samples == 0U || rate == 0U) {
    return false;
  }

  if (ebur_state != nullptr) {
    ebur128_destroy(&ebur_state);

    ebur_state = nullptr;
  }

  ebur_state = ebur128_init(
      2U, rate, EBUR128_MODE_S | EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_TRUE_PEAK | EBUR128_MODE_HISTOGRAM);

  ebur128_set_channel(ebur_state, 0U, EBUR128_LEFT);
  ebur128_set_channel(ebur_state, 1U, EBUR128_RIGHT);

  return ebur_state != nullptr;
}

void LevelMeter::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  ebur128_ready = false;

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (ebur128_ready) {
          return;
        }

        if (2U * static_cast<size_t>(n_samples) != data.size()) {
          data.resize(static_cast<size_t>(n_samples) * 2U);
        }

        auto status = init_ebur128();

        std::scoped_lock<std::mutex> lock(data_mutex);

        ebur128_ready = status;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void LevelMeter::process(std::span<float>& left_in,
                         std::span<float>& right_in,
                         std::span<float>& left_out,
                         std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  std::ranges::copy(left_in, left_out.begin());
  std::ranges::copy(right_in, right_out.begin());

  if (bypass || !ebur128_ready) {
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

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary)) {
    momentary = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm)) {
    shortterm = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global)) {
    global = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative)) {
    relative = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_range(ebur_state, &range)) {
    range = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_true_peak(ebur_state, 0U, &true_peak_L)) {
    true_peak_L = 0.0;
  }

  if (EBUR128_SUCCESS != ebur128_true_peak(ebur_state, 1U, &true_peak_R)) {
    true_peak_R = 0.0;
  }

  true_peak_L = util::linear_to_db(true_peak_L);
  true_peak_R = util::linear_to_db(true_peak_R);

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void LevelMeter::process([[maybe_unused]] std::span<float>& left_in,
                         [[maybe_unused]] std::span<float>& right_in,
                         [[maybe_unused]] std::span<float>& left_out,
                         [[maybe_unused]] std::span<float>& right_out,
                         [[maybe_unused]] std::span<float>& probe_left,
                         [[maybe_unused]] std::span<float>& probe_right) {}

auto LevelMeter::get_latency_seconds() -> float {
  return 0.0F;
}

void LevelMeter::resetHistory() {
  setup();
}

float LevelMeter::getMomentaryLevel() const {
  return momentary;
}

float LevelMeter::getShorttermLevel() const {
  return shortterm;
}

float LevelMeter::getIntegratedLevel() const {
  return global;
}

float LevelMeter::getRelativeLevel() const {
  return relative;
}

float LevelMeter::getRangeLevel() const {
  return range;
}

float LevelMeter::getTruePeakL() const {
  return true_peak_L;
}

float LevelMeter::getTruePeakR() const {
  return true_peak_R;
}
