#include "crystalizer_ui.hpp"

CrystalizerUi::CrystalizerUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "crystalizer";

  // loading glade widgets

  builder->get_widget("mute_band0", mute_band0);
  builder->get_widget("mute_band1", mute_band1);
  builder->get_widget("mute_band2", mute_band2);
  builder->get_widget("mute_band3", mute_band3);
  builder->get_widget("mute_band4", mute_band4);

  get_object(builder, "intensity_band0", intensity_band0);
  get_object(builder, "intensity_band1", intensity_band1);
  get_object(builder, "intensity_band2", intensity_band2);
  get_object(builder, "intensity_band3", intensity_band3);
  get_object(builder, "intensity_band4", intensity_band4);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "freq1", freq1);
  get_object(builder, "freq2", freq2);
  get_object(builder, "freq3", freq3);
  get_object(builder, "freq4", freq4);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("mute-band0", mute_band0, "active", flag);
  settings->bind("mute-band1", mute_band1, "active", flag);
  settings->bind("mute-band2", mute_band2, "active", flag);
  settings->bind("mute-band3", mute_band3, "active", flag);
  settings->bind("mute-band4", mute_band4, "active", flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("intensity-band0", intensity_band0.get(), "value", flag);
  settings->bind("intensity-band1", intensity_band1.get(), "value", flag);
  settings->bind("intensity-band2", intensity_band2.get(), "value", flag);
  settings->bind("intensity-band3", intensity_band3.get(), "value", flag);
  settings->bind("intensity-band4", intensity_band4.get(), "value", flag);
  settings->bind("freq1", freq1.get(), "value", flag);
  settings->bind("freq2", freq2.get(), "value", flag);
  settings->bind("freq3", freq3.get(), "value", flag);
  settings->bind("freq4", freq4.get(), "value", flag);
}

CrystalizerUi::~CrystalizerUi() {
  util::debug(name + " ui destroyed");
}
