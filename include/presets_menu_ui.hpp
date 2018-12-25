#ifndef PRESETS_MENU_UI_HPP
#define PRESETS_MENU_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include "application.hpp"

class PresetsMenuUi : public Gtk::Grid {
 public:
  PresetsMenuUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const Glib::RefPtr<Gio::Settings>& refSettings,
                Application* application);

  virtual ~PresetsMenuUi();

 private:
  std::string log_tag = "spectrum_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  std::vector<sigc::connection> connections;
};

#endif
