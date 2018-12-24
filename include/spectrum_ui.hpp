#ifndef SPECTRUM_UI_HPP
#define SPECTRUM_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/switch.h>
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
};

#endif
