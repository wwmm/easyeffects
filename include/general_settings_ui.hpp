#ifndef GENERAL_SETTINGS_UI_HPP
#define GENERAL_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "application.hpp"

class GeneralSettingsUi : public Gtk::Grid {
 public:
  GeneralSettingsUi(BaseObjectType* cobject,
                    const Glib::RefPtr<Gtk::Builder>& builder,
                    const Glib::RefPtr<Gio::Settings>& refSettings,
                    Application* application);

  virtual ~GeneralSettingsUi();

  static GeneralSettingsUi* add_to_stack(Gtk::Stack* stack, Application* app) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/ui/general_settings.glade");

    auto settings = Gio::Settings::create("com.github.wwmm.pulseeffects");

    GeneralSettingsUi* ui;

    builder->get_widget_derived("widgets_grid", ui, settings, app);

    stack->add(*ui, "general_spectrum", _("General"));

    return ui;
  }

 private:
  std::string log_tag = "general_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Switch *enable_autostart, *enable_all_apps, *theme_switch;
  Gtk::Button *reset_settings, *about_button;

  std::vector<sigc::connection> connections;

  void init_autostart_switch();

  bool on_enable_autostart(bool state);

  void on_reset_settings();
};

#endif
