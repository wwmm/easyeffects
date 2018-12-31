#ifndef SPECTRUM_SETTINGS_UI_HPP
#define SPECTRUM_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/builder.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "application.hpp"

class SpectrumSettingsUi : public Gtk::Grid {
 public:
  SpectrumSettingsUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const Glib::RefPtr<Gio::Settings>& refSettings,
                     Application* application);

  virtual ~SpectrumSettingsUi();

  static void add_to_stack(Gtk::Stack* stack, Application* app);

 private:
  std::string log_tag = "spectrum_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Switch *show_spectrum, *use_custom_color, *spectrum_fill;
  Gtk::ColorButton* spectrum_color_button;

  Glib::RefPtr<Gtk::Adjustment> spectrum_n_points, spectrum_height,
      spectrum_scale, spectrum_exponent, spectrum_sampling_freq;

  std::vector<sigc::connection> connections;

  void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& name,
                  Glib::RefPtr<Gtk::Adjustment>& object) {
    object =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  bool on_show_spectrum(bool state);

  void on_spectrum_sampling_freq_set();

  bool on_use_custom_color(bool state);
};

#endif
