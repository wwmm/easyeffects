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

#ifndef CALIBRATION_MIC_UI_HPP
#define CALIBRATION_MIC_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/spinner.h>
#include <gtkmm/togglebutton.h>
#include "calibration_mic.hpp"

class CalibrationMicUi : public Gtk::Grid {
 public:
  CalibrationMicUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  CalibrationMicUi(const CalibrationMicUi&) = delete;
  auto operator=(const CalibrationMicUi&) -> CalibrationMicUi& = delete;
  CalibrationMicUi(const CalibrationMicUi&&) = delete;
  auto operator=(const CalibrationMicUi&&) -> CalibrationMicUi& = delete;
  ~CalibrationMicUi() override;

  std::unique_ptr<CalibrationMic> cm;

 private:
  std::string log_tag = "calibration_mic_ui: ";

  const double default_time_window = 2.0;  // seconds

  Gtk::Button *measure_noise = nullptr, *start = nullptr, *stop = nullptr;
  Gtk::ToggleButton* subtract_noise = nullptr;
  Gtk::Spinner* spinner = nullptr;
  Gtk::SpinButton* time_window = nullptr;
};

#endif
