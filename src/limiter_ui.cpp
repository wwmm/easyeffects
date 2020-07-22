#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "limiter";

  // loading glade widgets

  builder->get_widget("auto-level", auto_level);
  builder->get_widget("asc", asc);
  builder->get_widget("attenuation", attenuation);
  builder->get_widget("attenuation_label", attenuation_label);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "limit", limit);
  get_object(builder, "lookahead", lookahead);
  get_object(builder, "release", release);
  get_object(builder, "oversampling", oversampling);
  get_object(builder, "asc_level", asc_level);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("limit", limit.get(), "value", flag);
  settings->bind("lookahead", lookahead.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("oversampling", oversampling.get(), "value", flag);
  settings->bind("auto-level", auto_level, "active", flag);
  settings->bind("asc", asc, "active", flag);
  settings->bind("asc-level", asc_level.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

LimiterUi::~LimiterUi() {
  util::debug(name + " ui destroyed");
}

void LimiterUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".limiter.input-gain");

    update_default_key<double>(settings, "limit", section + ".limiter.limit");

    update_default_key<double>(settings, "lookahead", section + ".limiter.lookahead");

    update_default_key<double>(settings, "release", section + ".limiter.release");

    update_default_key<bool>(settings, "auto-level", section + ".limiter.auto-level");

    update_default_key<bool>(settings, "asc", section + ".limiter.asc");

    update_default_key<double>(settings, "asc-level", section + ".limiter.asc-level");

    update_default_key<int>(settings, "oversampling", section + ".limiter.oversampling");

    update_default_key<double>(settings, "output-gain", section + ".limiter.output-gain");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void LimiterUi::on_new_attenuation(double value) {
  attenuation->set_value(1 - value);

  attenuation_label->set_text(level_to_str(util::linear_to_db(static_cast<float>(value)), 0));
}
