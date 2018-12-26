#ifndef GENERAL_SETTINGS_UI_HPP
#define GENERAL_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/grid.h>
#include <gtkmm/switch.h>

class GeneralSettingsUi : public Gtk::Grid {
 public:
  GeneralSettingsUi(BaseObjectType* cobject,
                    const Glib::RefPtr<Gtk::Builder>& builder,
                    const Glib::RefPtr<Gio::Settings>& refSettings);

  virtual ~GeneralSettingsUi();

 private:
  std::string log_tag = "general_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Switch *enable_autostart, *enable_all_apps, *theme_switch;

  std::vector<sigc::connection> connections;
};

#endif
