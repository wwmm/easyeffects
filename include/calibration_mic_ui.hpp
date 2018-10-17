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
  CalibrationMicUi(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& builder);

  virtual ~CalibrationMicUi();

  std::unique_ptr<CalibrationMic> cm;

 private:
  std::string log_tag = "calibration_mic_ui: ";

  Gtk::Button* measure_noise;
  Gtk::ToggleButton* subtract_noise;
  Gtk::Spinner* spinner;
  Gtk::SpinButton* time_window;
};

#endif
