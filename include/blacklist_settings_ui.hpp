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

#include "preset_type.hpp"

class BlacklistSettingsUi : public Gtk::Grid {
 public:
  BlacklistSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  BlacklistSettingsUi(const BlacklistSettingsUi&) = delete;
  auto operator=(const BlacklistSettingsUi&) -> BlacklistSettingsUi& = delete;
  BlacklistSettingsUi(const BlacklistSettingsUi&&) = delete;
  auto operator=(const BlacklistSettingsUi &&) -> BlacklistSettingsUi& = delete;
  ~BlacklistSettingsUi() override;

  static void add_to_stack(Gtk::Stack* stack);
  static bool add_new_entry(Glib::RefPtr<Gio::Settings> settings, const std::string& name, PresetType preset_type);
  static void remove_entry(Glib::RefPtr<Gio::Settings> settings, const std::string& name, PresetType preset_type);

 private:
  std::string log_tag = "blacklist_settings_ui: ";

  Glib::RefPtr<Gio::Settings> settings;

  Gtk::Button *add_blacklist_in = nullptr, *add_blacklist_out = nullptr;
  Gtk::ListBox *blacklist_in_listbox = nullptr, *blacklist_out_listbox = nullptr;
  Gtk::Entry *blacklist_in_name = nullptr, *blacklist_out_name = nullptr;
  Gtk::ScrolledWindow *blacklist_in_scrolled_window = nullptr, *blacklist_out_scrolled_window = nullptr;

  std::vector<sigc::connection> connections;

  void populate_blacklist_in_listbox();

  void populate_blacklist_out_listbox();

  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;
};

#endif
