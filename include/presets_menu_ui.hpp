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

class PresetsMenuUi : public Gtk::Grid {
 public:
  PresetsMenuUi(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder,
                const Glib::RefPtr<Gio::Settings>& refSettings,
                Application* application);

  virtual ~PresetsMenuUi();

  static PresetsMenuUi* add_to_popover(Gtk::Popover* popover, Application* app);

  void on_presets_menu_button_clicked();

 private:
  std::string log_tag = "presets_menu_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Button *add_preset, *import_preset;
  Gtk::ListBox* presets_listbox;

  Gtk::Entry* preset_name;
  Gtk::ScrolledWindow* presets_scrolled_window;

  std::vector<sigc::connection> connections;

  void populate_presets_listbox();

  int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);

  void on_import_preset_clicked();
};

#endif
