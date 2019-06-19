#include "exciter_ui.hpp"

ExciterUi::ExciterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& settings_name)
    : Gtk::Grid(cobject), PluginUiBase(builder, settings_name) {
  name = "exciter";

  // loading glade widgets

  builder->get_widget("harmonics_levelbar", harmonics_levelbar);
  builder->get_widget("harmonics_levelbar_label", harmonics_levelbar_label);
  builder->get_widget("ceil_active", ceil_active);
  builder->get_widget("listen", listen);

  get_object(builder, "amount", amount);
  get_object(builder, "blend", blend);
  get_object(builder, "ceil", ceilv);
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
  settings->bind("ceil", ceilv.get(), "value", flag);
  settings->bind("blend", blend.get(), "value", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);
  settings->bind("listen", listen, "active", flag);
  settings->bind("ceil-active", ceil_active, "active", flag);
}

ExciterUi::~ExciterUi() {
  util::debug(name + " ui destroyed");
}

void ExciterUi::on_new_harmonics_level(double value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
