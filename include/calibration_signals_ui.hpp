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

#ifndef CALIBRATION_SIGNALS_UI_HPP
#define CALIBRATION_SIGNALS_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include "calibration_signals.hpp"

class CalibrationSignalsUi : public Gtk::Box {
 public:
  CalibrationSignalsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  CalibrationSignalsUi(const CalibrationSignalsUi&) = delete;
  auto operator=(const CalibrationSignalsUi&) -> CalibrationSignalsUi& = delete;
  CalibrationSignalsUi(const CalibrationSignalsUi&&) = delete;
  auto operator=(const CalibrationSignalsUi&&) -> CalibrationSignalsUi& = delete;
  ~CalibrationSignalsUi() override;

  std::unique_ptr<CalibrationSignals> cs;

 private:
  std::string log_tag = "calibration_signals_ui: ";

  Gtk::Switch* enable = nullptr;
  Gtk::Scale* volume = nullptr;
  Gtk::SpinButton* frequency = nullptr;
  Gtk::ComboBoxText* wave = nullptr;
};

#endif
