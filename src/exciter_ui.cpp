/*
 *  Copyright © 2017-2020 Wellington Wallace
 *
 *  This file is part of PulseEffects.
 *
 *  PulseEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PulseEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with PulseEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "exciter_ui.hpp"

ExciterUi::ExciterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "exciter";

  // loading glade widgets

  builder->get_widget("harmonics_levelbar", harmonics_levelbar);
  builder->get_widget("harmonics_levelbar_label", harmonics_levelbar_label);
  builder->get_widget("ceil_active", ceil_active);
  builder->get_widget("ceil_freq", ceil_freq);
  builder->get_widget("listen", listen);
  builder->get_widget("plugin_reset", reset_button);

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
  settings->bind("ceil-active", ceil_freq, "sensitive", Gio::SettingsBindFlags::SETTINGS_BIND_GET);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

ExciterUi::~ExciterUi() {
  util::debug(name + " ui destroyed");
}

void ExciterUi::reset() {
  try {
    std::string section = (preset_type == PresetType::output) ? "output" : "input";

    update_default_key<double>(settings, "input-gain", section + ".exciter.input-gain");

    update_default_key<double>(settings, "output-gain", section + ".exciter.output-gain");

    update_default_key<double>(settings, "amount", section + ".exciter.amount");

    update_default_key<double>(settings, "harmonics", section + ".exciter.harmonics");

    update_default_key<double>(settings, "scope", section + ".exciter.scope");

    update_default_key<double>(settings, "ceil", section + ".exciter.ceil");

    update_default_key<double>(settings, "blend", section + ".exciter.blend");

    update_default_key<bool>(settings, "ceil-active", section + ".exciter.ceil-active");

    update_default_key<bool>(settings, "listen", section + ".exciter.listen");

    util::debug(name + " plugin: successfully reset");
  } catch (std::exception& e) {
    util::debug(name + " plugin: an error occurred during reset process");
  }
}

void ExciterUi::on_new_harmonics_level(double value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
