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

#include "bass_enhancer.hpp"
#include <algorithm>
#include <format>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_bass_enhancer.h"
#include "lv2_macros.hpp"
#include "lv2_wrapper.hpp"
#include "pipeline_type.hpp"
#include "plugin_base.hpp"
#include "pw_manager.hpp"
#include "tags_plugin_name.hpp"
#include "util.hpp"

BassEnhancer::BassEnhancer(const std::string& tag,
                           pw::Manager* pipe_manager,
                           PipelineType pipe_type,
                           QString instance_id)
    : PluginBase(tag,
                 tags::plugin_name::BaseName::bassEnhancer,
                 tags::plugin_package::Package::calf,
                 instance_id,
                 pipe_manager,
                 pipe_type),
      settings(db::Manager::self().get_plugin_db<db::BassEnhancer>(
          pipe_type,
          tags::plugin_name::BaseName::bassEnhancer + "#" + instance_id)) {
  const auto lv2_plugin_uri = "http://calf.sourceforge.net/plugins/BassEnhancer";

  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>(lv2_plugin_uri);

  packageInstalled = lv2_wrapper->found_plugin;

  if (!packageInstalled) {
    util::debug(std::format("{}{} is not installed", log_tag, lv2_plugin_uri));
  }

  init_common_controls<db::BassEnhancer>(settings);

  // specific plugin controls

  BIND_LV2_PORT("listen", listen, setListen, db::BassEnhancer::listenChanged);
  BIND_LV2_PORT_DB("amount", amount, setAmount, db::BassEnhancer::amountChanged, false);
  BIND_LV2_PORT("drive", harmonics, setHarmonics, db::BassEnhancer::harmonicsChanged);
  BIND_LV2_PORT("freq", scope, setScope, db::BassEnhancer::scopeChanged);
  BIND_LV2_PORT("floor", floor, setFloor, db::BassEnhancer::floorChanged);
  BIND_LV2_PORT("blend", blend, setBlend, db::BassEnhancer::blendChanged);
  BIND_LV2_PORT("floor_active", floorActive, setFloorActive, db::BassEnhancer::floorActiveChanged);
}

BassEnhancer::~BassEnhancer() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  settings->disconnect();

  util::debug(std::format("{}{} destroyed", log_tag, name.toStdString()));
}

void BassEnhancer::reset() {
  settings->setDefaults();
}

void BassEnhancer::setup() {
  if (!lv2_wrapper->found_plugin) {
    return;
  }

  lv2_wrapper->set_n_samples(n_samples);

  if (lv2_wrapper->get_rate() != rate) {
    lv2_wrapper->create_instance(rate);
  }
}

void BassEnhancer::process(std::span<float>& left_in,
                           std::span<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::ranges::copy(left_in, left_out.begin());
    std::ranges::copy(right_in, right_out.begin());

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

    harmonics_port_value = util::linear_to_db(lv2_wrapper->get_control_port_value("meter_drive"));
  }
}

void BassEnhancer::process([[maybe_unused]] std::span<float>& left_in,
                           [[maybe_unused]] std::span<float>& right_in,
                           [[maybe_unused]] std::span<float>& left_out,
                           [[maybe_unused]] std::span<float>& right_out,
                           [[maybe_unused]] std::span<float>& probe_left,
                           [[maybe_unused]] std::span<float>& probe_right) {}

auto BassEnhancer::get_latency_seconds() -> float {
  return 0.0F;
}

float BassEnhancer::getHarmonicsLevel() const {
  return harmonics_port_value;
}
