#include "maximizer_ui.hpp"

MaximizerUi::MaximizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "maximizer";

  // loading glade widgets

  builder->get_widget("reduction", reduction);
  builder->get_widget("reduction_label", reduction_label);

  get_object(builder, "ceiling", ceiling);
  get_object(builder, "release", release);
  get_object(builder, "threshold", threshold);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("ceiling", ceiling.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("threshold", threshold.get(), "value", flag);
}

MaximizerUi::~MaximizerUi() {
  util::debug(name + " ui destroyed");
}

void MaximizerUi::on_new_reduction(double value) {
  reduction->set_value(value);

  reduction_label->set_text(level_to_str(value, 0));
}
