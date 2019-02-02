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
  builder->get_widget("scale_low", scale_low);
  builder->get_widget("scale_mid", scale_mid);
  builder->get_widget("scale_high", scale_high);
  builder->get_widget("mute_icon_low", mute_icon_low);
  builder->get_widget("mute_icon_mid", mute_icon_mid);
  builder->get_widget("mute_icon_high", mute_icon_high);

  get_object(builder, "intensity_low", intensity_low);
  get_object(builder, "intensity_mid", intensity_mid);
  get_object(builder, "intensity_high", intensity_high);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);
  get_object(builder, "volume_low", volume_low);
  get_object(builder, "volume_mid", volume_mid);
  get_object(builder, "volume_high", volume_high);

  mute_low->signal_toggled().connect([&]() {
    if (mute_low->get_active()) {
      mute_icon_low->set_from_icon_name("audio-volume-muted-symbolic",
                                        Gtk::ICON_SIZE_BUTTON);
    } else {
      mute_icon_low->set_from_icon_name("audio-volume-high-symbolic",
                                        Gtk::ICON_SIZE_BUTTON);
    }
  });

  mute_mid->signal_toggled().connect([&]() {
    if (mute_mid->get_active()) {
      mute_icon_mid->set_from_icon_name("audio-volume-muted-symbolic",
                                        Gtk::ICON_SIZE_BUTTON);
    } else {
      mute_icon_mid->set_from_icon_name("audio-volume-high-symbolic",
                                        Gtk::ICON_SIZE_BUTTON);
    }
  });

  mute_high->signal_toggled().connect([&]() {
    if (mute_high->get_active()) {
      mute_icon_high->set_from_icon_name("audio-volume-muted-symbolic",
                                         Gtk::ICON_SIZE_BUTTON);
    } else {
      mute_icon_high->set_from_icon_name("audio-volume-high-symbolic",
                                         Gtk::ICON_SIZE_BUTTON);
    }
  });

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
  auto inv_flag = Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("mute-low", mute_low, "active", flag);
  settings->bind("mute-mid", mute_mid, "active", flag);
  settings->bind("mute-high", mute_high, "active", flag);

  settings->bind("mute-low", scale_low, "sensitive", inv_flag);
  settings->bind("mute-mid", scale_mid, "sensitive", inv_flag);
  settings->bind("mute-high", scale_high, "sensitive", inv_flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("intensity-low", intensity_low.get(), "value", flag);
  settings->bind("intensity-mid", intensity_mid.get(), "value", flag);
  settings->bind("intensity-high", intensity_high.get(), "value", flag);
  settings->bind("volume-low", volume_low.get(), "value", flag);
  settings->bind("volume-mid", volume_mid.get(), "value", flag);
  settings->bind("volume-high", volume_high.get(), "value", flag);

  settings->set_boolean("post-messages", true);
}

CrystalizerUi::~CrystalizerUi() {
  settings->set_boolean("post-messages", false);

  util::debug(name + " ui destroyed");
}
