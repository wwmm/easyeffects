#ifndef APPLICATION_WINDOW_HPP
#define APPLICATION_WINDOW_HPP

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/popover.h>
#include <gtkmm/stack.h>
#include "application.hpp"
#include "calibration_ui.hpp"
#include "presets_menu_ui.hpp"
#include "pulse_info_ui.hpp"
#include "sink_input_effects_ui.hpp"
#include "source_output_effects_ui.hpp"

class ApplicationUi : public Gtk::ApplicationWindow {
 public:
  ApplicationUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Application* application);

  virtual ~ApplicationUi();

  static ApplicationUi* create(Application* app);

 private:
  std::string log_tag = "application_ui: ";

  Application* app;

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Button *calibration_button, *help_button;
  Gtk::Stack *stack, *stack_menu_settings;
  Gtk::Label* headerbar_info;
  Gtk::Popover* presets_menu;
  Gtk::MenuButton* presets_menu_button;
  Gtk::Label* presets_menu_label;

  Gtk::Grid* subtitle_grid;
  Gtk::HeaderBar* headerbar;
  Gtk::Image *headerbar_icon1, *headerbar_icon2;

  std::vector<sigc::connection> connections;

  PresetsMenuUi* presets_menu_ui;

  SinkInputEffectsUi* sie_ui;
  SourceOutputEffectsUi* soe_ui;
  PulseInfoUi* pulse_info_ui;

  int sie_latency = 0, soe_latency = 0;

  void get_object(const Glib::RefPtr<Gtk::Builder>& builder,
                  const std::string& name,
                  Glib::RefPtr<Gtk::Adjustment>& object) {
    object = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object(name));
  }

  void update_headerbar_subtitle(const int& index);

  void apply_css_style(std::string css_file_name);

  void on_stack_visible_child_changed();

  void on_calibration_button_clicked();
};

#endif
