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

  time_window->set_value(2.0);
}

CalibrationMicUi::~CalibrationMicUi() {
  util::debug(log_tag + "destroyed");
}
