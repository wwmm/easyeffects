#include "presets_menu_ui.hpp"
#include "util.hpp"

PresetsMenuUi::PresetsMenuUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const Glib::RefPtr<Gio::Settings>& refSettings,
                             Application* application)
    : Gtk::Grid(cobject), settings(refSettings), app(application) {
  // loading glade widgets

  // builder->get_widget("spectrum", spectrum);

  // signals connection

  // auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;
}

PresetsMenuUi::~PresetsMenuUi() {
  for (auto c : connections) {
    c.disconnect();
  }

  util::debug(log_tag + "destroyed");
}
