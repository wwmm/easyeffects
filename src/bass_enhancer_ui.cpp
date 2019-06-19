#include "bass_enhancer_ui.hpp"

BassEnhancerUi::BassEnhancerUi(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder,
                               const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "bass_enhancer";

  // loading glade widgets

  builder->get_widget("harmonics_levelbar", harmonics_levelbar);
  builder->get_widget("harmonics_levelbar_label", harmonics_levelbar_label);
  builder->get_widget("floor_active", floor_active);
  builder->get_widget("listen", listen);

  get_object(builder, "amount", amount);
  get_object(builder, "blend", blend);
  get_object(builder, "floor", floorv);
  get_object(builder, "harmonics", harmonics);
  get_object(builder, "scope", scope);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("amount", amount.get(), "value", flag);
  settings->bind("harmonics", harmonics.get(), "value", flag);
  settings->bind("scope", scope.get(), "value", flag);
  settings->bind("floor", floorv.get(), "value", flag);
  settings->bind("blend", blend.get(), "value", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("listen", listen, "active", flag);
  settings->bind("floor-active", floor_active, "active", flag);
}

BassEnhancerUi::~BassEnhancerUi() {
  util::debug(name + " ui destroyed");
}

void BassEnhancerUi::on_new_harmonics_level(double value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
