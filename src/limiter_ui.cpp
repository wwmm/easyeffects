#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "limiter";

  // loading glade widgets

  builder->get_widget("asc", asc);
  builder->get_widget("attenuation", attenuation);
  builder->get_widget("attenuation_label", attenuation_label);

  get_object(builder, "input_gain", input_gain);
  get_object(builder, "limit", limit);
  get_object(builder, "lookahead", lookahead);
  get_object(builder, "release", release);
  get_object(builder, "oversampling", oversampling);
  get_object(builder, "asc_level", asc_level);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);

  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("limit", limit.get(), "value", flag);
  settings->bind("lookahead", lookahead.get(), "value", flag);
  settings->bind("release", release.get(), "value", flag);
  settings->bind("oversampling", oversampling.get(), "value", flag);
  settings->bind("asc", asc, "active", flag);
  settings->bind("asc-level", asc_level.get(), "value", flag);
}

LimiterUi::~LimiterUi() {
  util::debug(name + " ui destroyed");
}

void LimiterUi::on_new_attenuation(double value) {
  attenuation->set_value(1 - value);

  attenuation_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
