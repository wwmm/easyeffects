#include "loudness_ui.hpp"

LoudnessUi::LoudnessUi(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "loudness";

  // loading glade widgets

  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "loudness", loudness);
  get_object(builder, "output", output);
  get_object(builder, "link", link);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("loudness", loudness.get(), "value", flag);
  settings->bind("output", output.get(), "value", flag);
  settings->bind("link", link.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

LoudnessUi::~LoudnessUi() {
  util::debug(name + " ui destroyed");
}

void LoudnessUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "loudness", section + ".loudness.loudness");

    update_default_key<double>(settings, "output", section + ".loudness.output");

    update_default_key<double>(settings, "link", section + ".loudness.link");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}
