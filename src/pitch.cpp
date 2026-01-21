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

#include "pitch.hpp"
#include <qnamespace.h>
#include <qobject.h>
#include <soundtouch/STTypes.h>
#include <soundtouch/SoundTouch.h>
#include <algorithm>
#include <cstddef>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_pitch.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Pitch::Pitch(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::pitch,
                 tags::plugin_package::Package::soundTouch,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::Pitch>(pipe_type,
                                                            tags::plugin_name::BaseName::pitch + "#" + instance_id)) {
  init_common_controls<db::Pitch>(settings);

  dry = (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));

  wet = (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));

  // resetting soundtouch when bypass is pressed so its internal data is discarded

  connect(settings, &db::Pitch::bypassChanged, [&]() { resetHistory(); });

  connect(settings, &db::Pitch::quickSeekChanged, [&]() { set_quick_seek(); });

  connect(settings, &db::Pitch::antiAliasChanged, [&]() { set_anti_alias(); });

  connect(settings, &db::Pitch::sequenceLengthChanged, [&]() { set_sequence_length(); });

  connect(settings, &db::Pitch::seekWindowChanged, [&]() { set_seek_window(); });

  connect(settings, &db::Pitch::overlapLengthChanged, [&]() { set_overlap_length(); });

  connect(settings, &db::Pitch::tempoDifferenceChanged, [&]() { set_tempo_difference(); });

  connect(settings, &db::Pitch::rateDifferenceChanged, [&]() { set_rate_difference(); });

  connect(settings, &db::Pitch::octavesChanged, [&]() { set_semitones(); });

  connect(settings, &db::Pitch::semitonesChanged, [&]() { set_semitones(); });

  connect(settings, &db::Pitch::centsChanged, [&]() { set_semitones(); });

  connect(settings, &db::Pitch::dryChanged, [&]() {
    dry =
        (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));
  });

  connect(settings, &db::Pitch::wetChanged, [&]() {
    wet =
        (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));
  });
}

Pitch::~Pitch() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Pitch::reset() {
  settings->setDefaults();
}

void Pitch::clear_data() {
  setup();
}

void Pitch::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  soundtouch_ready = false;

  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        if (soundtouch_ready) {
          return;
        }

        latency_n_frames = 0U;

        if (data.size() != static_cast<size_t>(n_samples) * 2) {
          data.resize(2U * static_cast<size_t>(n_samples));
        }

        deque_out_L.resize(0U);
        deque_out_R.resize(0U);

        init_soundtouch();

        std::scoped_lock<std::mutex> lock(data_mutex);

        soundtouch_ready = true;
      },
      Qt::QueuedConnection);
}

void Pitch::process(std::span<float>& left_in,
                    std::span<float>& right_in,
                    std::span<float>& left_out,
                    std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (bypass || !soundtouch_ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  for (size_t n = 0U; n < left_in.size(); n++) {
    data[n * 2U] = left_in[n];
    data[(n * 2U) + 1U] = right_in[n];
  }

  snd_touch->putSamples(data.data(), n_samples);

  uint n_received = 0U;

  do {
    n_received = snd_touch->receiveSamples(data.data(), n_samples);

    for (size_t n = 0U; n < n_received; n++) {
      deque_out_L.push_back(data[n * 2U]);
      deque_out_R.push_back(data[(n * 2U) + 1U]);
    }

  } while (n_received != 0);

  if (deque_out_L.size() >= left_out.size()) {
    for (float& v : left_out) {
      v = deque_out_L.front();

      deque_out_L.pop_front();
    }

    for (float& v : right_out) {
      v = deque_out_R.front();

      deque_out_R.pop_front();
    }
  } else {
    const uint offset = left_out.size() - deque_out_L.size();

    if (offset != latency_n_frames) {
      latency_n_frames = offset;

      notify_latency = true;
    }

    for (uint n = 0U; !deque_out_L.empty() && n < left_out.size(); n++) {
      if (n < offset) {
        left_out[n] = 0.0F;
        right_out[n] = 0.0F;
      } else {
        left_out[n] = deque_out_L.front();
        right_out[n] = deque_out_R.front();

        deque_out_R.pop_front();
        deque_out_L.pop_front();
      }
    }
  }

  for (size_t n = 0; n < left_out.size(); n++) {
    left_out[n] = (wet * left_out[n]) + (dry * left_in[n]);

    right_out[n] = (wet * right_out[n]) + (dry * right_in[n]);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (notify_latency) {
    latency_value = static_cast<float>(latency_n_frames) / static_cast<float>(rate);

    util::debug(std::format("{}{} latency: {} s", log_tag, name.toStdString(), latency_value));

    update_filter_params();

    notify_latency = false;
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Pitch::process([[maybe_unused]] std::span<float>& left_in,
                    [[maybe_unused]] std::span<float>& right_in,
                    [[maybe_unused]] std::span<float>& left_out,
                    [[maybe_unused]] std::span<float>& right_out,
                    [[maybe_unused]] std::span<float>& probe_left,
                    [[maybe_unused]] std::span<float>& probe_right) {}

void Pitch::set_semitones() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setPitchSemiTones(settings->semitones() + (settings->octaves() * 12.0) + (settings->cents() / 100.0));
}

void Pitch::set_sequence_length() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_SEQUENCE_MS, settings->sequenceLength());
}

void Pitch::set_seek_window() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_SEEKWINDOW_MS, settings->seekWindow());
}

void Pitch::set_overlap_length() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_OVERLAP_MS, settings->overlapLength());
}

void Pitch::set_quick_seek() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_USE_QUICKSEEK, static_cast<int>(settings->quickSeek()));
}

void Pitch::set_anti_alias() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setSetting(SETTING_USE_AA_FILTER, static_cast<int>(settings->antiAlias()));
}

void Pitch::set_tempo_difference() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setTempoChange(settings->tempoDifference());
}

void Pitch::set_rate_difference() {
  if (snd_touch == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  snd_touch->setRateChange(settings->rateDifference());
}

void Pitch::init_soundtouch() {
  delete snd_touch;

  snd_touch = new soundtouch::SoundTouch();

  snd_touch->setSampleRate(rate);
  snd_touch->setChannels(2);

  set_semitones();
  set_quick_seek();
  set_anti_alias();
  set_sequence_length();
  set_seek_window();
  set_overlap_length();
  set_tempo_difference();
  set_rate_difference();
}

auto Pitch::get_latency_seconds() -> float {
  return latency_value;
}

void Pitch::resetHistory() {
  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

  QMetaObject::invokeMethod(baseWorker, [this] { setup(); }, Qt::QueuedConnection);

  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}
