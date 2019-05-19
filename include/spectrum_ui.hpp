#ifndef SPECTRUM_UI_HPP
#define SPECTRUM_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>

class SpectrumUi : public Gtk::Grid {
 public:
  SpectrumUi(BaseObjectType* cobject,
             const Glib::RefPtr<Gtk::Builder>& builder);

  virtual ~SpectrumUi();

  static SpectrumUi* add_to_box(Gtk::Box* box);

  void on_new_spectrum(const std::vector<float>& magnitudes);

  void clear_spectrum();

 private:
  std::string log_tag = "spectrum_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::DrawingArea* spectrum;
  Gdk::RGBA color, gradient_color;

  std::vector<sigc::connection> connections;

  bool mouse_inside = false;
  double mouse_intensity = 0, mouse_freq = 0;
  std::vector<float> spectrum_mag;

  bool on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx);

  bool on_spectrum_enter_notify_event(GdkEventCrossing* event);

  bool on_spectrum_leave_notify_event(GdkEventCrossing* event);

  bool on_spectrum_motion_notify_event(GdkEventMotion* event);

  void init_color();

  void init_gradient_color();
};

#endif
