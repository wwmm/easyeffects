/*
 *  Copyright © 2017-2021 Wellington Wallace
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

#ifndef SPECTRUM_HPP
#define SPECTRUM_HPP

#include <fftw3.h>
#include "plugin_base.hpp"

class Spectrum : public PluginBase {
 public:
  Spectrum(const std::string& tag,
           const std::string& schema,
           const std::string& schema_path,
           PipeManager* pipe_manager);
  Spectrum(const Spectrum&) = delete;
  auto operator=(const Spectrum&) -> Spectrum& = delete;
  Spectrum(const Spectrum&&) = delete;
  auto operator=(const Spectrum&&) -> Spectrum& = delete;
  ~Spectrum() override;

  void setup() override;

  void process(const std::vector<float>& left_in,
               const std::vector<float>& right_in,
               std::span<float>& left_out,
               std::span<float>& right_out) override;

  sigc::signal<void(float, float)> level;

 private:
  fftwf_plan plan_l, plan_r;

  std::vector<float> fft_left_in, fft_left_out, fft_right_in, fft_right_out;
};

#endif
