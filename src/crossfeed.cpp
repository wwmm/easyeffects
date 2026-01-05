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

#include "crossfeed.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_crossfeed.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Crossfeed::Crossfeed(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::crossfeed,
                 tags::plugin_package::Package::bs2b,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::Crossfeed>(
          pipe_type,
          tags::plugin_name::BaseName::crossfeed + "#" + instance_id)) {
  // bypass, input and output gain controls

  init_common_controls<db::Crossfeed>(settings);

  // specific plugin controls

  connect(settings, &db::Crossfeed::fcutChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    bs2b.set_level_feed(settings->fcut());
  });

  connect(settings, &db::Crossfeed::feedChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    bs2b.set_level_feed(10 * static_cast<int>(settings->feed()));
  });
}

Crossfeed::~Crossfeed() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Crossfeed::reset() {
  settings->setDefaults();
}

void Crossfeed::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  data.resize(2U * static_cast<size_t>(n_samples));

  if (rate != bs2b.get_srate()) {
    bs2b.set_srate(rate);
  }
}

void Crossfeed::process(std::span<float>& left_in,
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

  for (size_t n = 0U; n < left_in.size(); n++) {
    data[n * 2U] = left_in[n];
    data[(n * 2U) + 1U] = right_in[n];
  }

  bs2b.cross_feed(data.data(), n_samples);

  for (size_t n = 0U; n < left_out.size(); n++) {
    left_out[n] = data[n * 2U];
    right_out[n] = data[(n * 2U) + 1U];
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Crossfeed::process([[maybe_unused]] std::span<float>& left_in,
                        [[maybe_unused]] std::span<float>& right_in,
                        [[maybe_unused]] std::span<float>& left_out,
                        [[maybe_unused]] std::span<float>& right_out,
                        [[maybe_unused]] std::span<float>& probe_left,
                        [[maybe_unused]] std::span<float>& probe_right) {}

auto Crossfeed::get_latency_seconds() -> float {
  return 0.0F;
}
