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

#include "maximizer_ui.hpp"

MaximizerUi::MaximizerUi(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder,
                         const std::string& schema,
                         const std::string& schema_path)
    : Gtk::Grid(cobject), PluginUiBase(builder, schema, schema_path) {
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
  settings->reset("release");

  settings->reset("ceiling");

  settings->reset("threshold");
}

void MaximizerUi::on_new_reduction(double value) {
  reduction->set_value(value);

  reduction_label->set_text(level_to_str(value, 0));
}
