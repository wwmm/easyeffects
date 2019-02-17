#include "crystalizer_ui.hpp"

CrystalizerUi::CrystalizerUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "crystalizer";

  // loading glade widgets

  builder->get_widget("mute_low", mute_low);
  builder->get_widget("mute_mid", mute_mid);
  builder->get_widget("mute_high", mute_high);

  get_object(builder, "intensity_low", intensity_low);
  get_object(builder, "intensity_mid", intensity_mid);
  get_object(builder, "intensity_high", intensity_high);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "freq1", freq1);
  get_object(builder, "freq2", freq2);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("mute-low", mute_low, "active", flag);
  settings->bind("mute-mid", mute_mid, "active", flag);
  settings->bind("mute-high", mute_high, "active", flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("intensity-low", intensity_low.get(), "value", flag);
  settings->bind("intensity-mid", intensity_mid.get(), "value", flag);
  settings->bind("intensity-high", intensity_high.get(), "value", flag);
  settings->bind("freq1", freq1.get(), "value", flag);
  settings->bind("freq2", freq2.get(), "value", flag);
}

CrystalizerUi::~CrystalizerUi() {
  util::debug(name + " ui destroyed");
}
