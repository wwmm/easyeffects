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
#include <gtkmm/switch.h>
#include "preset_type.hpp"
#include "util.hpp"

class BlacklistSettingsUi : public Gtk::Grid {
 public:
  BlacklistSettingsUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  BlacklistSettingsUi(const BlacklistSettingsUi&) = delete;
  auto operator=(const BlacklistSettingsUi&) -> BlacklistSettingsUi& = delete;
  BlacklistSettingsUi(const BlacklistSettingsUi&&) = delete;
  auto operator=(const BlacklistSettingsUi &&) -> BlacklistSettingsUi& = delete;
  ~BlacklistSettingsUi() override;

  static void add_to_stack(Gtk::Stack* stack);

  // Blacklist management static methods
  static auto add_new_entry(const std::string& name, PresetType preset_type) -> bool;
  static void remove_entry(const std::string& name, PresetType preset_type);
  static auto app_is_blacklisted(const std::string& name, PresetType preset_type) -> bool;
  static auto get_blacklisted_apps_visibility() -> bool;

 private:
  std::string log_tag = "blacklist_settings_ui: ";

  Gtk::Switch* show_blacklisted_apps = nullptr;
  Gtk::Button *add_blacklist_in = nullptr, *add_blacklist_out = nullptr;
  static Gtk::ListBox *blacklist_in_listbox, *blacklist_out_listbox;
  Gtk::Entry *blacklist_in_name = nullptr, *blacklist_out_name = nullptr;
  Gtk::ScrolledWindow *blacklist_in_scrolled_window = nullptr, *blacklist_out_scrolled_window = nullptr;

  Glib::RefPtr<Gio::Settings> settings;

  // singleton pointer used to access settings from static methods
  // needed because Gio::Settings instance can't be made static
  static BlacklistSettingsUi* thisPtr;

  static std::vector<sigc::connection> connections;

  static void populate_blacklist_in_listbox();
  static void populate_blacklist_out_listbox();
  static auto on_listbox_sort(Gtk::ListBoxRow* row1, Gtk::ListBoxRow* row2) -> int;
};

#endif
