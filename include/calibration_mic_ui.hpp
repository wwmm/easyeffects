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
  auto operator=(const CalibrationMicUi &&) -> CalibrationMicUi& = delete;
  ~CalibrationMicUi() override;

  std::unique_ptr<CalibrationMic> cm;

 private:
  std::string log_tag = "calibration_mic_ui: ";

  const double default_time_window = 2.0;  // seconds

  Gtk::Button* measure_noise = nullptr;
  Gtk::ToggleButton* subtract_noise = nullptr;
  Gtk::Spinner* spinner = nullptr;
  Gtk::SpinButton* time_window = nullptr;
};

#endif
