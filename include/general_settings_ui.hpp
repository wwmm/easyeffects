#ifndef GENERAL_SETTINGS_UI_HPP
#define GENERAL_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "application.hpp"

class GeneralSettingsUi : public Gtk::Grid {
 public:
  GeneralSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Application* application);

  virtual ~GeneralSettingsUi();

  static void add_to_stack(Gtk::Stack* stack, Application* app);

 private:
  std::string log_tag = "general_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Switch *enable_autostart, *enable_all_sinkinputs, *enable_all_sourceoutputs, *theme_switch;
  Gtk::Button *reset_settings, *about_button;
  Gtk::SpinButton *realtime_priority_control, *niceness_control;
  Gtk::ComboBoxText* priority_type;

  Glib::RefPtr<Gtk::Adjustment> adjustment_priority, adjustment_niceness;

  std::vector<sigc::connection> connections;

  void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& name,
                  Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  void init_autostart_switch();

  bool on_enable_autostart(bool state);

  void on_reset_settings();

  void set_priority_controls_visibility();
};

#endif
