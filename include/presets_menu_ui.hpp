#ifndef PRESETS_MENU_UI_HPP
#define PRESETS_MENU_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/popover.h>
#include <gtkmm/scrolledwindow.h>
#include "application.hpp"
#include "preset_type.hpp"

class PresetsMenuUi : public Gtk::Grid {
 public:
  PresetsMenuUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const Glib::RefPtr<Gio::Settings>& refSettings,
                Application* application);

  virtual ~PresetsMenuUi();

  static PresetsMenuUi* add_to_popover(Gtk::Popover* popover, Application* app);

  void on_presets_menu_button_clicked();

  std::string build_device_name(PresetType preset_type, const std::string& device);

 private:
  std::string log_tag = "presets_menu_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Button *add_output, *add_input, *import_output, *import_input;
  Gtk::ListBox *output_listbox, *input_listbox;

  Gtk::Entry *output_name, *input_name;
  Gtk::ScrolledWindow *output_scrolled_window, *input_scrolled_window;

  std::vector<sigc::connection> connections;

  void create_preset(PresetType preset_type);

  void import_preset(PresetType preset_type);

  void populate_listbox(PresetType preset_type);

  int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

  void reset_menu_button_label();

  bool is_autoloaded(PresetType preset_type, const std::string& name);
};

#endif
