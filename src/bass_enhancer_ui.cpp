#include "bass_enhancer_ui.hpp"

BassEnhancerUi::BassEnhancerUi(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder,
                               const std::string& schema,
                               const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "bass_enhancer";

  // loading glade widgets

  builder->get_widget("harmonics_levelbar", harmonics_levelbar);
  builder->get_widget("harmonics_levelbar_label", harmonics_levelbar_label);
  builder->get_widget("floor_active", floor_active);
  builder->get_widget("floor_freq", floor_freq);
  builder->get_widget("listen", listen);
  builder->get_widget("plugin_reset", reset_button);

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
  settings->bind("floor-active", floor_freq, "sensitive", Gio::SettingsBindFlags::SETTINGS_BIND_GET);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

BassEnhancerUi::~BassEnhancerUi() {
  util::debug(name + " ui destroyed");
}

void BassEnhancerUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".bass_enhancer.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".bass_enhancer.output-gain");

    update_default_key<double>(settings, "amount", section + ".bass_enhancer.amount");

    update_default_key<double>(settings, "harmonics", section + ".bass_enhancer.harmonics");

    update_default_key<double>(settings, "scope", section + ".bass_enhancer.scope");

    update_default_key<double>(settings, "floor", section + ".bass_enhancer.floor");

    update_default_key<double>(settings, "blend", section + ".bass_enhancer.blend");

    update_default_key<bool>(settings, "floor-active", section + ".bass_enhancer.floor-active");

    update_default_key<bool>(settings, "listen", section + ".bass_enhancer.listen");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void BassEnhancerUi::on_new_harmonics_level(double value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}
