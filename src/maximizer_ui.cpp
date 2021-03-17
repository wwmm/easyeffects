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
  name = plugin_name::maximizer;

  // loading builder widgets

  reduction_levelbar = builder->get_widget<Gtk::LevelBar>("reduction_levelbar");
  release = builder->get_widget<Gtk::SpinButton>("release");
  threshold = builder->get_widget<Gtk::SpinButton>("threshold");
  ceiling = builder->get_widget<Gtk::SpinButton>("ceiling");
  reduction_label = builder->get_widget<Gtk::Label>("reduction_label");

  reset_button = builder->get_widget<Gtk::Button>("reset_button");

  // gsettings bindings

  settings->bind("installed", this, "sensitive");
  settings->bind("ceiling", ceiling->get_adjustment().get(), "value");
  settings->bind("release", release->get_adjustment().get(), "value");
  settings->bind("threshold", threshold->get_adjustment().get(), "value");

  reset_button->signal_clicked().connect([this]() { reset(); });
}

MaximizerUi::~MaximizerUi() {
  util::debug(name + " ui destroyed");
}

void MaximizerUi::reset() {
  settings->reset("bypass");

  settings->reset("release");

  settings->reset("ceiling");

  settings->reset("threshold");
}

void MaximizerUi::on_new_reduction(double value) {
  reduction_levelbar->set_value(value);

  reduction_label->set_text(level_to_localized_string(value, 0));
}
