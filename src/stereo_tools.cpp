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

#include "stereo_tools.hpp"
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <QApplication>
#include <algorithm>
#include <cstddef>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_stereo_tools.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

StereoTools::StereoTools(const std::string& tag, pw::Manager* pipe_manager, PipelineType pipe_type, QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::stereoTools,
                 tags::plugin_package::Package::calf,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::StereoTools>(
          pipe_type,
          tags::plugin_name::BaseName::stereoTools + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://calf.sourceforge.net/plugins/StereoTools";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::StereoTools>(settings);

  // specific plugin controls

  BIND_LV2_PORT("mode", mode, setMode, db::StereoTools::modeChanged);
  BIND_LV2_PORT("balance_in", balanceIn, setBalanceIn, db::StereoTools::balanceInChanged);
  BIND_LV2_PORT("balance_out", balanceOut, setBalanceOut, db::StereoTools::balanceOutChanged);
  BIND_LV2_PORT("sbal", sbal, setSbal, db::StereoTools::sbalChanged);
  BIND_LV2_PORT("mpan", mpan, setMpan, db::StereoTools::mpanChanged);
  BIND_LV2_PORT("stereo_base", stereoBase, setStereoBase, db::StereoTools::stereoBaseChanged);
  BIND_LV2_PORT("delay", delay, setDelay, db::StereoTools::delayChanged);
  BIND_LV2_PORT("sc_level", scLevel, setScLevel, db::StereoTools::scLevelChanged);
  BIND_LV2_PORT("stereo_phase", stereoPhase, setStereoPhase, db::StereoTools::stereoPhaseChanged);
  BIND_LV2_PORT("softclip", softclip, setSoftclip, db::StereoTools::softclipChanged);
  BIND_LV2_PORT("mutel", mutel, setMutel, db::StereoTools::mutelChanged);
  BIND_LV2_PORT("muter", muter, setMuter, db::StereoTools::muterChanged);
  BIND_LV2_PORT("phasel", phasel, setPhasel, db::StereoTools::phaselChanged);
  BIND_LV2_PORT("phaser", phaser, setPhaser, db::StereoTools::phaserChanged);
  BIND_LV2_PORT_DB("slev", slev, setSlev, db::StereoTools::slevChanged, false);
  BIND_LV2_PORT_DB("mlev", mlev, setMlev, db::StereoTools::mlevChanged, false);

  dry = (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));

  wet = (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));

  connect(settings, &db::StereoTools::dryChanged, [&]() {
    dry =
        (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));
  });

  connect(settings, &db::StereoTools::wetChanged, [&]() {
    wet =
        (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));
  });
}

StereoTools::~StereoTools() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void StereoTools::reset() {
  settings->setDefaults();
}

void StereoTools::setup() {
  std::scoped_lock<std::mutex> lock(data_mutex);

  if (!lv2_wrapper->found_plugin) {
    return;
  }

  ready = false;

  lv2_wrapper->set_n_samples(n_samples);

  // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
  QMetaObject::invokeMethod(
      QApplication::instance(),
      [this] {
        lv2_wrapper->create_instance(rate);

        std::scoped_lock<std::mutex> lock(data_mutex);

        ready = true;
      },
      Qt::QueuedConnection);
  // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void StereoTools::process(std::span<float>& left_in,
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

  for (size_t n = 0; n < left_out.size(); n++) {
    left_out[n] = (wet * left_out[n]) + (dry * left_in[n]);

    right_out[n] = (wet * right_out[n]) + (dry * right_in[n]);
  }

  if (output_gain != 1.0F) {
    apply_gain(left_out, right_out, output_gain);
  }

  if (updateLevelMeters) {
    get_peaks(left_in, right_in, left_out, right_out);
  }
}

void StereoTools::process([[maybe_unused]] std::span<float>& left_in,
                          [[maybe_unused]] std::span<float>& right_in,
                          [[maybe_unused]] std::span<float>& left_out,
                          [[maybe_unused]] std::span<float>& right_out,
                          [[maybe_unused]] std::span<float>& probe_left,
                          [[maybe_unused]] std::span<float>& probe_right) {}

auto StereoTools::get_latency_seconds() -> float {
  return 0.0F;
}
