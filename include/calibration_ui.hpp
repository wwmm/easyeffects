#ifndef CALIBRATION_UI_HPP
#define CALIBRATION_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/stack.h>
#include <gtkmm/window.h>
#include "calibration_mic_ui.hpp"
#include "calibration_signals_ui.hpp"

class CalibrationUi : public Gtk::Window {
 public:
  CalibrationUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& refBuilder);

  virtual ~CalibrationUi();

  static CalibrationUi* create();

  void set_source_monitor_name(const std::string& name);

 private:
  std::string log_tag = "calibration_ui: ";

  Glib::RefPtr<Gtk::Builder> builder;

  Gtk::Stack* stack;
  Gtk::DrawingArea* spectrum;
  Gtk::HeaderBar* headerbar;

  sigc::connection spectrum_connection;

  bool mouse_inside;
  double mouse_intensity, mouse_freq, min_mag, max_mag;
  std::vector<float> spectrum_mag;

  CalibrationSignalsUi* calibration_signals_ui;
  CalibrationMicUi* calibration_mic_ui;

  void on_new_spectrum(const std::vector<float>& magnitudes);

  bool on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

  bool on_spectrum_enter_notify_event(GdkEventCrossing* event);

  bool on_spectrum_leave_notify_event(GdkEventCrossing* event);

  bool on_spectrum_motion_notify_event(GdkEventMotion* event);

  void on_stack_visible_child_changed();
};

#endif
