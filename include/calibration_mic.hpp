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

#ifndef CALIBRATION_MIC_HPP
#define CALIBRATION_MIC_HPP

#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <iostream>
#include <vector>

class CalibrationMic {
 public:
  CalibrationMic();
  CalibrationMic(const CalibrationMic&) = delete;
  auto operator=(const CalibrationMic&) -> CalibrationMic& = delete;
  CalibrationMic(const CalibrationMic&&) = delete;
  auto operator=(const CalibrationMic&&) -> CalibrationMic& = delete;
  ~CalibrationMic();

  std::string log_tag = "calibration_mic: ";

  GstElement *pipeline = nullptr, *source = nullptr, *sink = nullptr, *spectrum = nullptr;

  uint min_spectrum_freq = 20U;     // Hz
  uint max_spectrum_freq = 20000U;  // Hz
  int spectrum_threshold = -120;    // dB
  uint spectrum_nbands = 3200U, spectrum_nfreqs = 0U;
  uint spectrum_npoints = 300U;  // number of points displayed
  float spline_f0 = 0.0F, spline_df = 0.0F;
  bool measure_noise = false, subtract_noise = false;
  std::vector<float> spectrum_freqs, spectrum_x_axis;
  std::vector<float> spectrum_mag_tmp, spectrum_mag, noise;

  sigc::signal<void, std::vector<float>> new_spectrum;
  sigc::signal<void> noise_measured;

  void start() const;
  void stop() const;
  void set_window(const double& value) const;
  void set_input_node_id(const uint& id) const;

 private:
  GstBus* bus = nullptr;
};

#endif
