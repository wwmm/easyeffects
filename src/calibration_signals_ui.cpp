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

#include "calibration_signals_ui.hpp"
#include "util.hpp"

CalibrationSignalsUi::CalibrationSignalsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Box(cobject), cs(std::make_unique<CalibrationSignals>()) {
  // loading glade widgets

  builder->get_widget("enable", enable);
  builder->get_widget("volume", volume);
  builder->get_widget("frequency", frequency);
  builder->get_widget("wave", wave);

  enable->signal_state_set().connect(
      [=](bool status) {
        if (status) {
          cs->start();
        } else {
          cs->stop();
        }

        return false;
      },
      false);

  g_object_bind_property(wave->gobj(), "active", cs->source, "wave", G_BINDING_DEFAULT);

  frequency->signal_value_changed().connect([=]() { cs->set_freq(frequency->get_value()); });

  volume->signal_value_changed().connect([=]() { cs->set_volume(volume->get_value()); });

  // default values

  wave->set_active(0);  // sine
  frequency->set_value(1000.0);
  volume->set_value(1.0);
}

CalibrationSignalsUi::~CalibrationSignalsUi() {
  util::debug(log_tag + "destroyed");
}
