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
      settings(db::Manager::self().get_plugin_db<DbStereoTools>(
          pipe_type,
          tags::plugin_name::BaseName::stereoTools + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://calf.sourceforge.net/plugins/StereoTools";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<DbStereoTools>(settings);

  // specific plugin controls

  BIND_LV2_PORT("mode", mode, setMode, DbStereoTools::modeChanged);
  BIND_LV2_PORT("balance_in", balanceIn, setBalanceIn, DbStereoTools::balanceInChanged);
  BIND_LV2_PORT("balance_out", balanceOut, setBalanceOut, DbStereoTools::balanceOutChanged);
  BIND_LV2_PORT("sbal", sbal, setSbal, DbStereoTools::sbalChanged);
  BIND_LV2_PORT("mpan", mpan, setMpan, DbStereoTools::mpanChanged);
  BIND_LV2_PORT("stereo_base", stereoBase, setStereoBase, DbStereoTools::stereoBaseChanged);
  BIND_LV2_PORT("delay", delay, setDelay, DbStereoTools::delayChanged);
  BIND_LV2_PORT("sc_level", scLevel, setScLevel, DbStereoTools::scLevelChanged);
  BIND_LV2_PORT("stereo_phase", stereoPhase, setStereoPhase, DbStereoTools::stereoPhaseChanged);
  BIND_LV2_PORT("softclip", softclip, setSoftclip, DbStereoTools::softclipChanged);
  BIND_LV2_PORT("mutel", mutel, setMutel, DbStereoTools::mutelChanged);
  BIND_LV2_PORT("muter", muter, setMuter, DbStereoTools::muterChanged);
  BIND_LV2_PORT("phasel", phasel, setPhasel, DbStereoTools::phaselChanged);
  BIND_LV2_PORT("phaser", phaser, setPhaser, DbStereoTools::phaserChanged);
  BIND_LV2_PORT_DB("slev", slev, setSlev, DbStereoTools::slevChanged, false);
  BIND_LV2_PORT_DB("mlev", mlev, setMlev, DbStereoTools::mlevChanged, false);

  dry = (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));

  wet = (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));

  connect(settings, &DbStereoTools::dryChanged, [&]() {
    dry =
        (settings->dry() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->dry()));
  });

  connect(settings, &DbStereoTools::wetChanged, [&]() {
    wet =
        (settings->wet() <= util::minimum_db_d_level) ? 0.0F : static_cast<float>(util::db_to_linear(settings->wet()));
  });
}

StereoTools::~StereoTools() {
  stop_worker();

  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void StereoTools::reset() {
  settings->setDefaults();
}

void StereoTools::clear_data() {
  if (lv2_wrapper == nullptr) {
    return;
  }

  {
    std::scoped_lock<std::mutex> lock(data_mutex);

    lv2_wrapper->destroy_instance();
  }

  setup();
}

void StereoTools::setup() {
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
