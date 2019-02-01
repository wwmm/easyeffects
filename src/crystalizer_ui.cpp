#include "crystalizer_ui.hpp"

CrystalizerUi::CrystalizerUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "crystalizer";

  // loading glade widgets

  get_object(builder, "split_frequency", split_frequency);
  get_object(builder, "intensity_low", intensity_low);
  get_object(builder, "intensity_mid", intensity_mid);
  get_object(builder, "intensity_high", intensity_high);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("split-frequency", split_frequency.get(), "value", flag);
  settings->bind("intensity-low", intensity_low.get(), "value", flag);
  settings->bind("intensity-mid", intensity_mid.get(), "value", flag);
  settings->bind("intensity-high", intensity_high.get(), "value", flag);

  settings->set_boolean("post-messages", true);
}

CrystalizerUi::~CrystalizerUi() {
  settings->set_boolean("post-messages", false);

  util::debug(name + " ui destroyed");
}
