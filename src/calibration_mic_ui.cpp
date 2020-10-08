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

#include "calibration_mic_ui.hpp"
#include "util.hpp"

CalibrationMicUi::CalibrationMicUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Grid(cobject), cm(std::make_unique<CalibrationMic>()) {
  // loading glade widgets

  builder->get_widget("measure_noise", measure_noise);
  builder->get_widget("subtract_noise", subtract_noise);
  builder->get_widget("time_window", time_window);
  builder->get_widget("spinner", spinner);

  measure_noise->signal_clicked().connect([=]() {
    cm->measure_noise = true;
    spinner->start();
  });

  subtract_noise->signal_toggled().connect([=]() { cm->subtract_noise = subtract_noise->get_active(); });

  time_window->signal_value_changed().connect([=]() { cm->set_window(time_window->get_value()); });

  cm->noise_measured.connect([=]() { spinner->stop(); });

  time_window->set_value(default_time_window);
}

CalibrationMicUi::~CalibrationMicUi() {
  util::debug(log_tag + "destroyed");
}
