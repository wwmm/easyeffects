/*
 *  Copyright © 2017-2022 Wellington Wallace
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

#ifndef CALIBRATION_UI_HPP
#define CALIBRATION_UI_HPP

#include <glibmm/i18n.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/stack.h>
#include <gtkmm/window.h>
#include "calibration_mic_ui.hpp"
#include "calibration_signals_ui.hpp"
#include "util.hpp"

class CalibrationUi : public Gtk::Window {
 public:
  CalibrationUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  CalibrationUi(const CalibrationUi&) = delete;
  auto operator=(const CalibrationUi&) -> CalibrationUi& = delete;
  CalibrationUi(const CalibrationUi&&) = delete;
  auto operator=(const CalibrationUi&&) -> CalibrationUi& = delete;
  ~CalibrationUi() override;

  static auto create() -> CalibrationUi*;

  void set_input_node_id(const uint& id);

 private:
  inline static const std::string log_tag = "calibration_ui: ";

  Gtk::Stack* stack = nullptr;
  Gtk::DrawingArea* spectrum = nullptr;
  Gtk::HeaderBar* headerbar = nullptr;

  sigc::connection spectrum_connection;

  bool mouse_inside = false;
  double mouse_intensity = 0.0, mouse_freq = 0.0, min_mag = 0.0, max_mag = 0.0;
  std::vector<float> spectrum_mag;

  CalibrationSignalsUi* calibration_signals_ui = nullptr;
  CalibrationMicUi* calibration_mic_ui = nullptr;

  void on_new_spectrum(std::vector<float> magnitudes);

  auto on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool;

  auto on_spectrum_enter_notify_event(GdkEventCrossing* event) -> bool;

  auto on_spectrum_leave_notify_event(GdkEventCrossing* event) -> bool;

  auto on_spectrum_motion_notify_event(GdkEventMotion* event) -> bool;

  void on_stack_visible_child_changed();
};

#endif
