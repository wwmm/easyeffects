#ifndef BLACKLIST_SETTINGS_UI_HPP
#define BLACKLIST_SETTINGS_UI_HPP

#include <giomm/settings.h>
#include <glibmm/i18n.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stack.h>

class BlacklistSettingsUi : public Gtk::Grid {
 public:
  BlacklistSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  virtual ~BlacklistSettingsUi();

  static void add_to_stack(Gtk::Stack* stack);

 private:
  std::string log_tag = "blacklist_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Button *add_blacklist_in, *add_blacklist_out;
  Gtk::ListBox *blacklist_in_listbox, *blacklist_out_listbox;
  Gtk::Entry *blacklist_in_name, *blacklist_out_name;
  Gtk::ScrolledWindow *blacklist_in_scrolled_window, *blacklist_out_scrolled_window;

  std::vector<sigc::connection> connections;

  void populate_blacklist_in_listbox();

  void populate_blacklist_out_listbox();

  int on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2);
};

#endif
