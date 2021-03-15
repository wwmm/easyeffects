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

#include "bass_enhancer_ui.hpp"

BassEnhancerUi::BassEnhancerUi(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder,
                               const std::string& schema,
                               const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "bass_enhancer";

  // loading glade widgets

  harmonics_levelbar = builder->get_widget<Gtk::LevelBar>("harmonics_levelbar");
  harmonics_levelbar_label = builder->get_widget<Gtk::Label>("harmonics_levelbar_label");
  floor_active = builder->get_widget<Gtk::ToggleButton>("floor_active");
  listen = builder->get_widget<Gtk::ToggleButton>("listen");
  floor_freq = builder->get_widget<Gtk::SpinButton>("floor_freq");

  reset_button = builder->get_widget<Gtk::Button>("reset_button");

  // get_object(builder, "amount", amount);
  // get_object(builder, "blend", blend);
  // get_object(builder, "floor", floorv);
  // get_object(builder, "harmonics", harmonics);
  // get_object(builder, "scope", scope);
  // get_object(builder, "input_gain", input_gain);
  // get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  settings->bind("installed", this, "sensitive");
  settings->bind("amount", amount.get(), "value");
  settings->bind("harmonics", harmonics.get(), "value");
  settings->bind("scope", scope.get(), "value");
  settings->bind("floor", floorv.get(), "value");
  settings->bind("blend", blend.get(), "value");
  settings->bind("input-gain", input_gain.get(), "value");
  settings->bind("output-gain", output_gain.get(), "value");
  settings->bind("listen", listen, "active");
  settings->bind("floor-active", floor_active, "active");
  settings->bind("floor-active", floor_freq, "sensitive", Gio::Settings::BindFlags::GET);

  // reset plugin
  reset_button->signal_clicked().connect([=, this]() { reset(); });
}

BassEnhancerUi::~BassEnhancerUi() {
  util::debug(name + " ui destroyed");
}

void BassEnhancerUi::reset() {
  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("amount");

  settings->reset("harmonics");

  settings->reset("scope");

  settings->reset("floor");

  settings->reset("blend");

  settings->reset("floor-active");

  settings->reset("listen");
}

void BassEnhancerUi::on_new_harmonics_level(double value) {
  harmonics_levelbar->set_value(value);

  harmonics_levelbar_label->set_text(level_to_localized_string(util::linear_to_db(value), 0));
}
