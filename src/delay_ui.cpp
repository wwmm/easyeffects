#include "delay_ui.hpp"

DelayUi::DelayUi(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "delay";

  // loading glade widgets

  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "time_l", time_l);
  get_object(builder, "time_r", time_r);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("time-l", time_l.get(), "value", flag);
  settings->bind("time-r", time_r.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

DelayUi::~DelayUi() {
  util::debug(name + " ui destroyed");
}

void DelayUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".delay.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".delay.output-gain");

    update_default_key<double>(settings, "time-l", section + ".delay.time-l");

    update_default_key<double>(settings, "time-r", section + ".delay.time-r");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}
