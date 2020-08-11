#ifndef SPECTRUM_UI_HPP
#define SPECTRUM_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>

class SpectrumUi : public Gtk::Grid {
 public:
  SpectrumUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  SpectrumUi(const SpectrumUi&) = delete;
  auto operator=(const SpectrumUi&) -> SpectrumUi& = delete;
  SpectrumUi(const SpectrumUi&&) = delete;
  auto operator=(const SpectrumUi &&) -> SpectrumUi& = delete;
  ~SpectrumUi() override;

  static auto add_to_box(Gtk::Box* box) -> SpectrumUi*;

  void on_new_spectrum(const std::vector<float>& magnitudes);

  void clear_spectrum();

 private:
  std::string log_tag = "spectrum_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::DrawingArea* spectrum = nullptr;
  Gdk::RGBA color, color_frequency_axis_labels, gradient_color;

  std::vector<sigc::connection> connections;

  bool mouse_inside = false;
  double mouse_intensity = 0.0, mouse_freq = 0.0;
  std::vector<float> spectrum_mag;

  auto on_spectrum_draw(const Cairo::RefPtr<Cairo::Context>& ctx) -> bool;

  auto on_spectrum_enter_notify_event(GdkEventCrossing* event) -> bool;

  auto on_spectrum_leave_notify_event(GdkEventCrossing* event) -> bool;

  auto on_spectrum_motion_notify_event(GdkEventMotion* event) -> bool;

  void init_color();

  void init_frequency_labels_color();

  void init_gradient_color();
};

#endif
