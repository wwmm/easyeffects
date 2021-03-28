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

#ifndef MULTIBAND_GATE_HPP
#define MULTIBAND_GATE_HPP

#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"

class MultibandGate : public PluginBase {
 public:
  MultibandGate(const std::string& tag,
                const std::string& schema,
                const std::string& schema_path,
                PipeManager* pipe_manager);
  MultibandGate(const MultibandGate&) = delete;
  auto operator=(const MultibandGate&) -> MultibandGate& = delete;
  MultibandGate(const MultibandGate&&) = delete;
  auto operator=(const MultibandGate&&) -> MultibandGate& = delete;
  ~MultibandGate() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  sigc::signal<void(double)> output0, output1, output2, output3, gating0, gating1, gating2, gating3;

 private:
  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;
};

#endif
