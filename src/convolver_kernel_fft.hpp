/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects
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

#pragma once

#include <fftw3.h>
#include <qlist.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <vector>

class ConvolverKernelFFT {
 public:
  ConvolverKernelFFT();
  ~ConvolverKernelFFT();

  auto calculate_fft(std::vector<float> kernel_L,
                     std::vector<float> kernel_R,
                     float kernel_rate,
                     int interp_points = 1000) -> void;

  auto clear_data() -> void;

  QList<QPointF> linear_L;
  QList<QPointF> linear_R;
  QList<QPointF> log_L;
  QList<QPointF> log_R;

 private:
  static auto apply_hanning_window(std::vector<float>& signal) -> void;

  static auto compute_fft_magnitude(std::vector<float>& kernel) -> std::vector<double>;

  static auto normalize_spectrum(std::vector<double>& spectrum) -> void;
};
