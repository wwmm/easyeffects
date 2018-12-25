#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stack.h>
#include <gtkmm/switch.h>
#include "application.hpp"
#include "calibration_ui.hpp"
#include "pulse_settings_ui.hpp"
#include "sink_input_effects_ui.hpp"
#include "source_output_effects_ui.hpp"
#include "spectrum_settings_ui.hpp"
#include "spectrum_ui.hpp"

class ApplicationUi : public Gtk::ApplicationWindow {
 public:
  ApplicationUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                Application* application);

  virtual ~ApplicationUi();

  static ApplicationUi* create(Application* app);

 private:
  std::string log_tag = "application_ui: ";

  Application* app;

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Switch *enable_autostart, *enable_all_apps, *theme_switch;

  Gtk::Box* placeholder_spectrum;
  Gtk::Button *reset_settings, *add_preset, *import_preset, *calibration_button,
      *help_button, *add_blacklist_in, *add_blacklist_out, *about_button;
  Gtk::Stack *stack, *stack_menu_settings;
  Gtk::ListBox *presets_listbox, *blacklist_in_listbox, *blacklist_out_listbox;
  Gtk::MenuButton* presets_menu_button;
  Gtk::Label *presets_menu_label, *headerbar_info;
  Gtk::Entry *preset_name, *blacklist_in_name, *blacklist_out_name;
  Gtk::ScrolledWindow *presets_scrolled_window, *blacklist_in_scrolled_window,
      *blacklist_out_scrolled_window;

  Gtk::HeaderBar* headerbar;
  Gtk::Image *headerbar_icon1, *headerbar_icon2;

  sigc::connection spectrum_connection;
  std::vector<sigc::connection> connections;

  SpectrumUi* spectrum_ui;
  SpectrumSettingsUi* spectrum_settings_ui;
  PulseSettingsUi* pulse_settings_ui;
  SinkInputEffectsUi* sie_ui;
  SourceOutputEffectsUi* soe_ui;

  int sie_latency = 0, soe_latency = 0;

  void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& name,
                  Glib::RefPtr<Gtk::Adjustment>& object) {
    object =
        Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  void update_headerbar_subtitle(const int& index);

  void apply_css_style(std::string css_file_name);

  void init_autostart_switch();

  void populate_presets_listbox();

  void populate_blacklist_in_listbox();

  void populate_blacklist_out_listbox();

  bool on_enable_autostart(bool state);

  void on_reset_settings();

  void on_stack_visible_child_changed();

  int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

  void on_presets_menu_button_clicked();

  void on_import_preset_clicked();

  void on_calibration_button_clicked();
};

#endif
