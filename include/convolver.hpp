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

#ifndef CONVOLVER_HPP
#define CONVOLVER_HPP

#include <zita-convolver.h>
#include <algorithm>
#include <mutex>
#include <ranges>
#include <sndfile.hh>
#include <vector>
#include "plugin_base.hpp"
#include "resampler.hpp"

class Convolver : public PluginBase {
 public:
  Convolver(const std::string& tag,
            const std::string& schema,
            const std::string& schema_path,
            PipeManager* pipe_manager);
  Convolver(const Convolver&) = delete;
  auto operator=(const Convolver&) -> Convolver& = delete;
  Convolver(const Convolver&&) = delete;
  auto operator=(const Convolver&&) -> Convolver& = delete;
  ~Convolver() override;

  void setup() override;

  void process(std::span<float>& left_in,
               std::span<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

 private:
  uint kernel_n_frames = 0;
  uint ir_width = 100;

  std::vector<float> interleaved_kernel, kernel_L, kernel_R;

  std::unique_ptr<Resampler> resampler_L, resampler_R;

  Convproc* conv = nullptr;

  std::mutex lock_guard_zita;

  void read_kernel_file();

  void apply_kernel_autogain();

  void set_kernel_stereo_width();
};

#endif
