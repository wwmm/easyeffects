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

#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& builder,
                     const std::string& schema,
                     const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "limiter";

  // loading glade widgets

  builder->get_widget("auto-level", auto_level);
  builder->get_widget("asc", asc);
  builder->get_widget("asc_grid", asc_grid);
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
  settings->bind("asc", asc_grid, "sensitive", Gio::SettingsBindFlags::SETTINGS_BIND_GET);
  settings->bind("asc-level", asc_level.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

LimiterUi::~LimiterUi() {
  util::debug(name + " ui destroyed");
}

void LimiterUi::reset() {
  settings->reset("input-gain");

  settings->reset("limit");

  settings->reset("lookahead");

  settings->reset("release");

  settings->reset("auto-level");

  settings->reset("asc");

  settings->reset("asc-level");

  settings->reset("oversampling");

  settings->reset("output-gain");
}

void LimiterUi::on_new_attenuation(double value) {
  attenuation->set_value(1.0 - value);

  attenuation_label->set_text(level_to_str(util::linear_to_db(value), 0));
}
