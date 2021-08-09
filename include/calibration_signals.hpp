/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects
 *
 *  EasyEffectsis free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffectsis distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CALIBRATION_SIGNALS_HPP
#define CALIBRATION_SIGNALS_HPP

#include <gst/gst.h>
#include <sigc++/sigc++.h>
#include <iostream>
#include <vector>

class CalibrationSignals {
 public:
  CalibrationSignals();
  CalibrationSignals(const CalibrationSignals&) = delete;
  auto operator=(const CalibrationSignals&) -> CalibrationSignals& = delete;
  CalibrationSignals(const CalibrationSignals&&) = delete;
  auto operator=(const CalibrationSignals&&) -> CalibrationSignals& = delete;
  ~CalibrationSignals();

  std::string log_tag = "calibration_signals: ";

  GstElement *pipeline = nullptr, *source = nullptr, *sink = nullptr, *spectrum = nullptr;

  uint min_spectrum_freq = 20U;     // Hz
  uint max_spectrum_freq = 20000U;  // Hz
  int spectrum_threshold = -120;    // dB
  uint spectrum_nbands = 3200U, spectrum_nfreqs = 0U;
  uint spectrum_npoints = 300U;  // number of points displayed
  float spline_f0 = 0.0F, spline_df = 0.0F;
  std::vector<float> spectrum_freqs, spectrum_x_axis;
  std::vector<float> spectrum_mag_tmp, spectrum_mag;

  sigc::signal<void, std::vector<float>> new_spectrum;

  void start() const;
  void stop() const;
  void set_freq(const double& value) const;
  void set_volume(const double& value) const;

 private:
  GstBus* bus = nullptr;
};

#endif
