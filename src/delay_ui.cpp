#include "delay_ui.hpp"

DelayUi::DelayUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "delay";

  // loading glade widgets

  // get_object(builder, "intensity", intensity);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  // settings->bind("intensity", intensity.get(), "value", flag);

  settings->set_boolean("post-messages", true);
}

DelayUi::~DelayUi() {
  settings->set_boolean("post-messages", false);

  util::debug(name + " ui destroyed");
}
