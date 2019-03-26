#include "loudness_ui.hpp"

LoudnessUi::LoudnessUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "loudness";

  // loading glade widgets

  get_object(builder, "loudness", loudness);
  get_object(builder, "output", output);
  get_object(builder, "link", link);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("loudness", loudness.get(), "value", flag);
  settings->bind("output", output.get(), "value", flag);
  settings->bind("link", link.get(), "value", flag);
}

LoudnessUi::~LoudnessUi() {
  util::debug(name + " ui destroyed");
}
