#ifndef BLACKLIST_SETTINGS_UI_HPP
#define BLACKLIST_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include "application.hpp"

class BlacklistSettingsUi : public Gtk::Grid {
 public:
  BlacklistSettingsUi(BaseObjectType* cobject,
                      const Glib::RefPtr<Gtk::Builder>& builder,
                      const Glib::RefPtr<Gio::Settings>& refSettings,
                      Application* application);

  virtual ~BlacklistSettingsUi();

 private:
  std::string log_tag = "blacklist_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Application* app;

  Gtk::Button *add_blacklist_in, *add_blacklist_out;
  Gtk::ListBox *blacklist_in_listbox, *blacklist_out_listbox;
  Gtk::Entry *blacklist_in_name, *blacklist_out_name;
  Gtk::ScrolledWindow *blacklist_in_scrolled_window,
      *blacklist_out_scrolled_window;

  std::vector<sigc::connection> connections;

  void populate_blacklist_in_listbox();

  void populate_blacklist_out_listbox();

  int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);
};

#endif
