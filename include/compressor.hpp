/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include "lv2_wrapper.hpp"
#include "plugin_base.hpp"

class Compressor : public PluginBase {
 public:
  Compressor(const std::string& tag,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager);
  Compressor(const Compressor&) = delete;
  auto operator=(const Compressor&) -> Compressor& = delete;
  Compressor(const Compressor&&) = delete;
  auto operator=(const Compressor&&) -> Compressor& = delete;
  ~Compressor() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out,
               std::span<float>& probe_left,
               std::span<float>& probe_right) override;

  sigc::signal<void(double)> reduction, sidechain, curve, latency;

 private:
  uint latency_n_frames = 0;

  std::unique_ptr<lv2::Lv2Wrapper> lv2_wrapper;

  std::vector<pw_proxy*> list_proxies;
};

#endif
