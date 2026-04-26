/**
 * Copyright © 2017-2026 Wellington Wallace
 * Crosstalk Canceller plugin developed by Antti S. Lankila <alankila@bel.fi>
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

#include "crosstalk_canceller.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_crosstalk_canceller.h"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

CrosstalkCanceller::CrosstalkCanceller(const std::string& tag,
                                       pw::Manager* pipe_manager,
                                       PipelineType pipe_type,
                                       QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::crosstalkCanceller,
                 tags::plugin_package::Package::ee,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<DbCrosstalkCanceller>(
          pipe_type,
          tags::plugin_name::BaseName::crosstalkCanceller + "#" + instance_id)) {
  // bypass, input and output gain controls

  init_common_controls<DbCrosstalkCanceller>(settings);

  // specific plugin controls

  connect(settings, &DbCrosstalkCanceller::delayUsChanged, [&]() {
    std::scoped_lock<std::mutex> lock(data_mutex);

    a.configure(settings->delayUs(), rate);
    b.configure(settings->delayUs(), rate);
  });
}

CrosstalkCanceller::~CrosstalkCanceller() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void CrosstalkCanceller::reset() {
  settings->setDefaults();
}

void CrosstalkCanceller::clear_data() {
  setup();
}

void CrosstalkCanceller::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  data.resize(2U * static_cast<size_t>(n_samples));

  a.configure(settings->delayUs(), rate);
  b.configure(settings->delayUs(), rate);
}

void CrosstalkCanceller::process(std::span<float>& left_in,
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

  const auto decay_gain = static_cast<float>(std::pow(10, settings->decayDb() / 20));

  if (settings->phantomCenterOnly()) {
    for (size_t n = 0U; n < left_in.size(); n++) {
      const float middle = left_in[n] + right_in[n];
      const float side = left_in[n] - right_in[n];
      const auto mo = middle - (decay_gain * a.get_sample());
      const auto so = side;
      left_out[n] = (mo + so) * .5f;
      right_out[n] = (mo - so) * .5f;
      a.put_sample(mo);
      /* No delay line input from b */
    }
  } else {
    for (size_t n = 0U; n < left_in.size(); n++) {
      const auto ao = left_in[n] - (decay_gain * b.get_sample());
      const auto bo = right_in[n] - (decay_gain * a.get_sample());
      left_out[n] = ao;
      right_out[n] = bo;
      a.put_sample(ao);
      b.put_sample(bo);
    }
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void CrosstalkCanceller::process([[maybe_unused]] std::span<float>& left_in,
                                 [[maybe_unused]] std::span<float>& right_in,
                                 [[maybe_unused]] std::span<float>& left_out,
                                 [[maybe_unused]] std::span<float>& right_out,
                                 [[maybe_unused]] std::span<float>& probe_left,
                                 [[maybe_unused]] std::span<float>& probe_right) {}

auto CrosstalkCanceller::get_latency_seconds() -> float {
  return 0.0F;
}
