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

#include "pitch_ui.hpp"

PitchUi::PitchUi(BaseObjectType* cobject,
                 const Glib::RefPtr<Gtk::Builder>& builder,
                 const std::string& schema,
                 const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = "pitch";

  // loading glade widgets

  builder->get_widget("faster", faster);
  builder->get_widget("formant_preserving", formant_preserving);
  builder->get_widget("plugin_reset", reset_button);

  get_object(builder, "cents", cents);
  get_object(builder, "crispness", crispness);
  get_object(builder, "semitones", semitones);
  get_object(builder, "octaves", octaves);
  get_object(builder, "input_gain", input_gain);
  get_object(builder, "output_gain", output_gain);

  // gsettings bindings

  auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;

  settings->bind("installed", this, "sensitive", flag);
  settings->bind("cents", cents.get(), "value", flag);
  settings->bind("crispness", crispness.get(), "value", flag);
  settings->bind("semitones", semitones.get(), "value", flag);
  settings->bind("octaves", octaves.get(), "value", flag);
  settings->bind("faster", faster, "active", flag);
  settings->bind("formant-preserving", formant_preserving, "active", flag);
  settings->bind("input-gain", input_gain.get(), "value", flag);
  settings->bind("output-gain", output_gain.get(), "value", flag);

  // reset plugin
  reset_button->signal_clicked().connect([=]() { reset(); });
}

PitchUi::~PitchUi() {
  util::debug(name + " ui destroyed");
}

void PitchUi::reset() {
  settings->reset("input-gain");

  settings->reset("output-gain");

  settings->reset("cents");

  settings->reset("semitones");

  settings->reset("octaves");

  settings->reset("crispness");

  settings->reset("formant-preserving");

  settings->reset("faster");
}
