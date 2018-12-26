#include "general_settings_ui.hpp"
#include "util.hpp"

GeneralSettingsUi::GeneralSettingsUi(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder,
    const Glib::RefPtr<Gio::Settings>& refSettings)
    : Gtk::Grid(cobject), settings(refSettings) {
  // loading glade widgets

  // builder->get_widget("blacklist_in_scrolled_window",
  //                     blacklist_in_scrolled_window);

  // signals connection
}

GeneralSettingsUi::~GeneralSettingsUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}
