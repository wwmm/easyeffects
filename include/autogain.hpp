/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AUTOGAIN_HPP
#define AUTOGAIN_HPP

#include <ebur128.h>
#include <mutex>
#include "plugin_base.hpp"

class AutoGain : public PluginBase {
 public:
  AutoGain(const std::string& tag,
           const std::string& schema,
           const std::string& schema_path,
           PipeManager* pipe_manager);
  AutoGain(const AutoGain&) = delete;
  auto operator=(const AutoGain&) -> AutoGain& = delete;
  AutoGain(const AutoGain&&) = delete;
  auto operator=(const AutoGain&&) -> AutoGain& = delete;
  ~AutoGain() override;

  void setup() override;

  void process(const std::vector<float>& left_in,
               const std::vector<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  // loudness, gain, momentary, shortterm, integrated, relative, range;

  sigc::signal<void(double, double, double, double, double, double, double)> results;

 private:
  bool bypass = false;

  double target = -23.0;  // target loudness level
  double output_gain = 1.0;

  float notification_time_window = 0.1F;  // 100 ms
  float notification_dt = 0.0F;

  std::vector<float> data;

  ebur128_state* ebur_state = nullptr;

  std::mutex my_lock_guard;

  void init_ebur128();
};

#endif
