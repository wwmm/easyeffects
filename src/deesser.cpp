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

#include "deesser.hpp"
#include <qnamespace.h>
#include <qobject.h>
#include <QApplication>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_deesser.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

Deesser::Deesser(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::deesser,
                 tags::plugin_package::Package::calf,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(
          db::Manager::self().get_plugin_db<db::Deesser>(pipe_type,
                                                         tags::plugin_name::BaseName::deesser + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://calf.sourceforge.net/plugins/Deesser";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::Deesser>(settings);

  BIND_LV2_PORT("mode", mode, setMode, db::Deesser::modeChanged);
  BIND_LV2_PORT("detection", detection, setDetection, db::Deesser::detectionChanged);
  BIND_LV2_PORT("ratio", ratio, setRatio, db::Deesser::ratioChanged);
  BIND_LV2_PORT("f1_freq", f1Freq, setF1Freq, db::Deesser::f1FreqChanged);
  BIND_LV2_PORT("f2_freq", f2Freq, setF2Freq, db::Deesser::f2FreqChanged);
  BIND_LV2_PORT("f2_q", f2Q, setF2Q, db::Deesser::f2QChanged);
  BIND_LV2_PORT("laxity", laxity, setLaxity, db::Deesser::laxityChanged);
  BIND_LV2_PORT("sc_listen", scListen, setScListen, db::Deesser::scListenChanged);
  BIND_LV2_PORT_DB("threshold", threshold, setThreshold, db::Deesser::thresholdChanged, false);
  BIND_LV2_PORT_DB("makeup", makeup, setMakeup, db::Deesser::makeupChanged, false);
  BIND_LV2_PORT_DB("f1_level", f1Level, setF1Level, db::Deesser::f1LevelChanged, false);
  BIND_LV2_PORT_DB("f2_level", f2Level, setF2Level, db::Deesser::f2LevelChanged, false);
}

Deesser::~Deesser() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}
void Deesser::reset() {
  settings->setDefaults();
}

void Deesser::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void Deesser::setup() {
  if (rate == 0 || n_samples == 0) {
    // Some signals may be emitted before PipeWire calls our setup function
    return;
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->has_instance() && rate == lv2_wrapper->get_rate()) {
    return;
  }

  ready = false;

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

void Deesser::process(std::span<float>& left_in,
                      std::span<float>& right_in,
                      std::span<float>& left_out,
                      std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

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

  if (input_gain != 1.0F) {
    apply_gain(left_in, right_in, input_gain);
  }

  lv2_wrapper->connect_data_ports(left_in, right_in, left_out, right_out);
  lv2_wrapper->run();

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);

    compression_value = util::linear_to_db(lv2_wrapper->get_control_port_value("compression"));
    detected_value = util::linear_to_db(lv2_wrapper->get_control_port_value("detected"));
  }
}

void Deesser::process([[maybe_unused]] std::span<float>& left_in,
                      [[maybe_unused]] std::span<float>& right_in,
                      [[maybe_unused]] std::span<float>& left_out,
                      [[maybe_unused]] std::span<float>& right_out,
                      [[maybe_unused]] std::span<float>& probe_left,
                      [[maybe_unused]] std::span<float>& probe_right) {}

auto Deesser::get_latency_seconds() -> float {
  return 0.0F;
}

float Deesser::getCompressionLevel() const {
  return compression_value;
}

float Deesser::getDetectedLevel() const {
  return detected_value;
}
