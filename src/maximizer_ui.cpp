#include "maximizer_ui.hpp"

MaximizerUi::MaximizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "maximizer";

  // loading glade widgets

  builder->get_widget("reduction", reduction);
  builder->get_widget("reduction_label", reduction_label);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "ceiling", ceiling);
  get_object(builder, "release", release);
  get_object(builder, "threshold", threshold);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("ceiling", ceiling.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

MaximizerUi::~MaximizerUi() {
  util::debug(name + " ui destroyed");
}

void MaximizerUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "release", section + "maximizer.release");

    update_default_key<double>(settings, "ceiling", section + "maximizer.ceiling");
    
    update_default_key<double>(settings, "threshold", section + "maximizer.threshold");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void MaximizerUi::on_new_reduction(double value) {
  reduction->set_value(value);

  reduction_label->set_text(level_to_str(value, 0));
}
