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

#include "crusher.hpp"
#include <qapplication.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_crusher.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Crusher::Crusher(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::crusher,
                 tags::plugin_package::Package::calf,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Crusher>(pipe_type,
                                                         tags::plugin_name::BaseName::crusher + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://calf.sourceforge.net/plugins/Crusher";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Crusher>(settings);

  // specific plugin controls

  BIND_LV2_PORT("mode", mode, setMode, db::Crusher::modeChanged);
  BIND_LV2_PORT("bits", bitReduction, setBitReduction, db::Crusher::bitReductionChanged);
  BIND_LV2_PORT("morph", morph, setMorph, db::Crusher::morphChanged);
  BIND_LV2_PORT("anti_aliasing", antiAliasing, setAntiAliasing, db::Crusher::antiAliasingChanged);
  BIND_LV2_PORT("samples", sampleReduction, setSampleReduction, db::Crusher::sampleReductionChanged);
  BIND_LV2_PORT("lfo", lfoActive, setLfoActive, db::Crusher::lfoActiveChanged);
  BIND_LV2_PORT("lforange", lfoRange, setLfoRange, db::Crusher::lfoRangeChanged);
  BIND_LV2_PORT("lforate", lfoRate, setLfoRate, db::Crusher::lfoRateChanged);
  BIND_LV2_PORT_DB("dc", dc, setDc, db::Crusher::dcChanged, false);
}

Crusher::~Crusher() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void Crusher::reset() {
  settings->setDefaults();
}

void Crusher::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void Crusher::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  ready = false;

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      baseWorker,
      [this] {
        lv2_wrapper->create_instance(rate);

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Crusher::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  if (bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    return;
  }

  if (!ready) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

    if (output_gain != 1.0F) {
      apply_gain(left_out, right_out, output_gain);
    }

    return;
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void Crusher::process([[maybe_unused]] std::span<float>& left_in,
                      [[maybe_unused]] std::span<float>& right_in,
                      [[maybe_unused]] std::span<float>& left_out,
                      [[maybe_unused]] std::span<float>& right_out,
                      [[maybe_unused]] std::span<float>& probe_left,
                      [[maybe_unused]] std::span<float>& probe_right) {}

auto Crusher::get_latency_seconds() -> float {
  return 0.0F;
}
