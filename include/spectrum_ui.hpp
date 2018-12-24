#ifndef SPECTRUM_UI_HPP
#define SPECTRUM_UI_HPP

#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include "application.hpp"

class SpectrumUi : public Gtk::Grid {
 public:
  SpectrumUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder,
             const Glib::RefPtr<Gio::Settings>& refSettings,
             Application* application);

  virtual ~SpectrumUi();

 private:
  std::string log_tag = "spectrum_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Grid* widgets_grid;
  Gtk::DrawingArea* spectrum;
  Gdk::RGBA spectrum_color;

  std::vector<sigc::connection> connections;

  bool mouse_inside = false;
  double mouse_intensity = 0, mouse_freq = 0;
  std::vector<float> spectrum_mag;

  void clear_spectrum();

  void on_new_spectrum(const std::vector<float>& magnitudes);

  bool on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

  bool on_spectrum_enter_notify_event(GdkEventCrossing* event);

  bool on_spectrum_leave_notify_event(GdkEventCrossing* event);

  bool on_spectrum_motion_notify_event(GdkEventMotion* event);
};

#endif
