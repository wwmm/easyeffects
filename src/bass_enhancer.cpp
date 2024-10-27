/*
 *  Copyright © 2017-2024 Wellington Wallace
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

#include "bass_enhancer.hpp"
#include <algorithm>
#include <memory>
#include <span>
#include <string>
#include "db_manager.hpp"
#include "easyeffects_db_bass_enhancer.h"
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
  lv2_wrapper = std::make_unique<lv2::Lv2Wrapper>("http://calf.sourceforge.net/plugins/BassEnhancer");

  package_installed = lv2_wrapper->found_plugin;

  if (!package_installed) {
    util::debug(log_tag + "http://calf.sourceforge.net/plugins/BassEnhancer is not installed");
  }

  init_common_controls<db::BassEnhancer>(settings);

  // specific plugin controls

  lv2_wrapper->set_control_port_value("listen", static_cast<float>(settings->listen()));

  lv2_wrapper->sync_funcs.emplace_back([&]() { settings->setListen(lv2_wrapper->get_control_port_value("listen")); });

  connect(settings, &db::BassEnhancer::listenChanged,
          [&]() { lv2_wrapper->set_control_port_value("listen", static_cast<float>(settings->listen())); });

  //   lv2_wrapper->bind_key_double_db<"amount", "amount">(settings);

  //   lv2_wrapper->bind_key_double<"drive", "harmonics">(settings);

  //   lv2_wrapper->bind_key_double<"freq", "scope">(settings);

  //   lv2_wrapper->bind_key_double<"floor", "floor">(settings);

  //   lv2_wrapper->bind_key_double<"blend", "blend">(settings);

  //   lv2_wrapper->bind_key_bool<"floor_active", "floor-active">(settings);

  //   lv2_wrapper->bind_key_bool<"listen", "listen">(settings);

  // bypass, input and output gain controls
}

BassEnhancer::~BassEnhancer() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  util::debug(log_tag + name.toStdString() + " destroyed");
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

    // lv2_wrapper->load_ui();
  }
}

void BassEnhancer::process(std::span<float>& left_in,
                           std::span<float>& right_in,
                           std::span<float>& left_out,
                           std::span<float>& right_out) {
  if (!lv2_wrapper->found_plugin || !lv2_wrapper->has_instance() || bypass) {
    std::copy(left_in.begin(), left_in.end(), left_out.begin());
    std::copy(right_in.begin(), right_in.end(), right_out.begin());

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

  get_peaks(left_in, right_in, left_out, right_out);

  harmonics_port_value = util::linear_to_db(lv2_wrapper->get_control_port_value("meter_drive"));
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